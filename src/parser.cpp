#include <file.hpp>
#include <unicode.hpp>
#include <parser.hpp>
#include <tokenizer.hpp>
#include <detail/globals.hpp>

#include <bits/stdc++.h>

#include <boost/filesystem.hpp>

#include <magic_enum.hpp>

#include <simdutf.h>

GLOBAL_GETTER_DECL(
    std::vector<std::shared_ptr<const stdfs::path>>,
    already_processed_files
);

GLOBAL_GETTER_DECL(
    std::vector<stdfs::path>, supdef_import_paths
);

GLOBAL_XXHASHER_DECL(std::u32string, 64, u32string);
GLOBAL_XXHASHER_DECL(std::u16string, 64, u16string);
GLOBAL_XXHASHER_DECL(std::u8string, 64, u8string);
GLOBAL_XXHASHER_DECL(std::string, 64, string);
GLOBAL_XXHASHER_DECL(std::wstring, 64, wstring);
GLOBAL_XXHASHER_DECL(stdfs::path, 64, path);

supdef::parser::parser(const stdfs::path& filename)
    : m_file(stdfs::canonical(filename)), m_tokens(), m_imported_parsers(), m_supdefs(32, ::supdef::globals::get_xxhasher<supdef_map_type::key_type, 64UL>())
{
    ::supdef::globals::get_already_processed_files().push_back(m_file.filename());
}

supdef::parser::parser(stdfs::path&& filename)
    : m_file(stdfs::canonical(std::move(filename))), m_tokens(), m_imported_parsers(), m_supdefs(32, ::supdef::globals::get_xxhasher<supdef_map_type::key_type, 64UL>())
{
    ::supdef::globals::get_already_processed_files().push_back(m_file.filename());
}

supdef::parser::~parser()
{
}

namespace
{
    static std::string format(const std::u32string& str, size_t start = 0, size_t end = std::u32string::npos)
    {
        const auto& dataptr = str.data();
        const auto& datalen = str.size();
        const auto& datastart = dataptr + start;

        if (end == std::u32string::npos)
            end = datalen;
        
        if (str.empty())
            return "";

        size_t required_length = simdutf::utf8_length_from_utf32(datastart, end - start);
        std::unique_ptr<char[]> buf(new char[required_length]);
        size_t result = simdutf::convert_valid_utf32_to_utf8(datastart, end - start, buf.get());
        if (result == 0)
            throw std::runtime_error("failed to convert utf32 to utf8");

        return std::string(buf.get(), result);
    }

}

// tokenize the file
void ::supdef::parser::do_stage1()
{
    auto& data = m_file.data();
    auto filename = m_file.filename();
    
    tokenizer tk(data, filename);
    for (auto&& token : tk.tokenize())
        m_tokens.push_back(token);
}

// splice lines
void ::supdef::parser::do_stage2()
{
    auto& orig_data = m_file.original_data();
    auto token = m_tokens.begin();
    while (token != m_tokens.end())
    {
        if (token->kind == token_kind::backslash)
        {
            auto next_token = std::next(token);
            if (next_token == m_tokens.end())
            {
                printer::error(
                    "unexpected end of file while parsing backslash character",
                    *token, orig_data, &format
                );
                return;
            }
            if (next_token->kind == token_kind::newline)
            {
                m_tokens.erase(next_token);
                token->kind = token_kind::horizontal_whitespace;
                token->data = U" ";
                token->loc.toksize = 1;
            }
            else
            {
                printer::warning(
                    "backslash character outside of string or character literal",
                    *token, orig_data, &format
                );
            }
        }
        std::advance(token, 1);
    }
}

// remove comments
void ::supdef::parser::do_stage3()
{
    for (auto token = m_tokens.begin(); token != m_tokens.end(); std::advance(token, 1))
    {
        switch (token->kind)
        {
        case token_kind::inline_comment: {
            auto nltok = std::find_if(
                token, m_tokens.end(),
                [](const ::supdef::token& tok) {
                    return tok.kind == token_kind::newline;
                }
            );
            m_tokens.erase(token, nltok);
            token = std::prev(nltok);
        } break;
        case token_kind::multiline_comment: {
            token->data = U" ";
            token->kind = token_kind::horizontal_whitespace;
            token->loc.toksize = 1;
        } break;
        default:
            break;
        }
    }
}

namespace
{
    template <typename IterT>
    concept points_to_token = std::same_as<typename std::iterator_traits<IterT>::value_type, ::supdef::token>;

    template <typename IterT>
    concept points_to_token_and_bidir = points_to_token<IterT> && std::bidirectional_iterator<IterT>;

    template <typename IterT>
    concept points_to_token_and_fwd = points_to_token<IterT> && std::forward_iterator<IterT>;

    template <typename IterT>
    concept points_to_token_and_input = points_to_token<IterT> && std::input_iterator<IterT>;

    template <typename FnT, typename IterT>
    concept predicate = std::predicate<FnT, ::supdef::token> ||
                        std::predicate<FnT, IterT>;
    
    template <typename FnT, typename IterT>
    concept invokeable = std::invocable<FnT, ::supdef::token> ||
                         std::invocable<FnT, IterT>;

    static bool at_start_of_line(const points_to_token_and_bidir auto iter, const points_to_token_and_bidir auto begin)
    {
        auto cpy = iter;
        if (cpy == begin)
            return true;
        std::advance(cpy, -1);
        while (cpy != begin && cpy->kind != ::supdef::token_kind::newline)
        {
            if (cpy->kind != ::supdef::token_kind::horizontal_whitespace)
                return false;
            std::advance(cpy, -1);
        }
        return true;
    }

    static size_t skipws(points_to_token_and_input auto& iter, const points_to_token_and_input auto end, bool skip_newlines = false)
    {
        size_t count = 0;
        while (iter != end &&
                (iter->kind == ::supdef::token_kind::horizontal_whitespace ||
                    (skip_newlines && iter->kind == ::supdef::token_kind::newline)))
        {
            std::advance(iter, 1);
            count++;
        }
        return count;
    }

    template <typename IterT, typename PredT>
        requires points_to_token_and_input<IterT> && predicate<PredT, IterT>
    static size_t skip_until(IterT& iter, const IterT end, PredT pred)
    {
        size_t count = 0;
        if constexpr (std::invocable<PredT, IterT>)
        {
            while (iter != end && !pred(iter))
            {
                std::advance(iter, 1);
                count++;
            }
        }
        else
        {
            while (iter != end && !pred(*iter))
            {
                std::advance(iter, 1);
                count++;
            }
        }
        return count;
    }

    template <typename IterT>
        requires points_to_token_and_input<IterT>
    static size_t skip_until(IterT& iter, const IterT end, ::supdef::token_kind kind)
    {
        return skip_until(iter, end, [kind](const ::supdef::token& tok) {
            return tok.kind == kind;
        });
    }

    template <typename IterT, typename PredT, typename FnT>
        requires points_to_token_and_input<IterT> && predicate<PredT, IterT> && invokeable<FnT&&, IterT>
    static size_t skip_until(IterT& iter, const IterT end, PredT pred, FnT&& fn)
    {
        size_t count = 0;
        FnT fncpy = std::forward<FnT>(fn);
        if constexpr (std::invocable<PredT, IterT>)
        {
            while (iter != end && !pred(iter))
            {
                if constexpr (std::invocable<FnT&&, IterT>)
                    std::invoke(fncpy, iter);
                else
                    std::invoke(fncpy, *iter);
                std::advance(iter, 1);
                count++;
            }
        }
        else
        {
            while (iter != end && !pred(*iter))
            {
                if constexpr (std::invocable<FnT&&, IterT>)
                    std::invoke(fncpy, iter);
                else
                    std::invoke(fncpy, *iter);
                std::advance(iter, 1);
                count++;
            }
        }
        return count;
    }
    
    template <typename IterT, typename FnT>
        requires points_to_token_and_input<IterT> && invokeable<FnT&&, IterT>
    static size_t skip_until(IterT& iter, const IterT end, ::supdef::token_kind kind, FnT&& fn)
    {
        return skip_until(iter, end, [kind](const ::supdef::token& tok) {
            return tok.kind == kind;
        }, std::forward<FnT>(fn));
    }
}

// process imports
void ::supdef::parser::do_stage4()
{
    std::string errmsg;
    ::supdef::token errtok;
    const auto& orig_data = m_file.original_data();

    for (auto token = m_tokens.cbegin(); token != m_tokens.cend(); std::advance(token, 1))
    {
        if (token->kind != token_kind::at)
            continue;
        if (!at_start_of_line(token, m_tokens.cbegin()))
            continue;

        const auto import_start = token;
        std::remove_const_t<decltype(import_start)> import_end;

        std::advance(token, 1);
        skipws(token, m_tokens.cend());
        if (token->kind != token_kind::keyword)
            continue;
        if (token->keyword != keyword_kind::import)
            continue;

        std::advance(token, 1);
        if (skipws(token, m_tokens.cend()) == 0)
            printer::warning(
                "missing whitespace after import keyword",
                *token, orig_data, &format
            );
        if (token == m_tokens.cend())
        {
            errmsg = "unexpected end of file while parsing import statement";
            errtok = *std::prev(token);
            goto error;
        }

        stdfs::path extracted_path;
        token_kind pathkind = token->kind;
        switch (token->kind)
        {
        case token_kind::string_literal:
            extracted_path = std::move(token->data.value());
            skip_until(token, m_tokens.cend(), token_kind::newline);
            import_end = token;
            break;
        case token_kind::langle: {
            const auto langle = token;
            std::u32string tmppath;
            
            std::advance(token, 1);
            skip_until(
                token, m_tokens.cend(), token_kind::rangle,
                [&tmppath](const ::supdef::token& tok) {
                    tmppath += tok.data.value();
                }
            );
            if (token == m_tokens.cend())
            {
                errmsg = "unexpected end of file while parsing import statement";
                errtok = *langle;
                goto error;
            }

            extracted_path = std::move(tmppath);
            skip_until(token, m_tokens.cend(), token_kind::newline);
            import_end = token;
        } break;
        default:
            errmsg = "unexpected token while parsing import statement";
            errtok = *token;
            goto fatal;
        }

        if (extracted_path.empty())
        {
            errmsg = "empty path in import statement";
            errtok = *import_start;
            goto fatal;
        }

        m_tokens.erase(import_start, import_end);

        if (!this->add_child_parser(extracted_path, pathkind))
        {
            errmsg = "failed to import file `" + extracted_path.string() + "`: file not found";
            errtok = *import_start;
            goto error;
        }
    }

    goto end;

fatal:
    printer::fatal(
        errmsg, errtok, orig_data, &format
    );
    ::exit(EXIT_FAILURE);

error:
    printer::error(
        errmsg, errtok, orig_data, &format
    );

end:
    for (parser& p : m_imported_parsers)
    {
        p.do_stage1();
        p.do_stage2();
        p.do_stage3();
        p.do_stage4();
        p.do_stage5();
    }
}

std::optional<
    std::pair<
        ::supdef::parser::registered_supdef::options, std::u32string
    >
> supdef::parser::parse_supdef_start(
    std::list<token>::const_iterator line_start,
    std::list<token>::const_iterator line_end,
    const std::u32string& origdata
)
{
    using ret_type = std::optional<std::pair<registered_supdef::options, std::u32string>>;

    registered_supdef::options opts;
    std::u32string name;
    size_t skipped;
    auto tok = line_start;

    skipws(tok, line_end);
    if (tok == line_end || tok->kind != token_kind::at)
        return std::nullopt;

    std::advance(tok, 1);
    skipws(tok, line_end);
    if (tok == line_end || tok->kind != token_kind::keyword || tok->keyword != keyword_kind::supdef)
        return std::nullopt;

    std::advance(tok, 1);
    skipped = skipws(tok, line_end);
    if (tok == line_end)
        return std::nullopt;
    if (skipped == 0)
        printer::warning(
            "missing whitespace after @supdef keyword",
            *tok, origdata, &format
        );
    
    if (tok->kind == token_kind::keyword && tok->keyword == keyword_kind::begin)
        opts = registered_supdef::options::none;
    else
    {
        std::u32string optstring;
        skip_until(
            tok, line_end, token_kind::horizontal_whitespace,
            [&optstring](const ::supdef::token& tok) {
                optstring += tok.data.value();
            }
        );
        if (tok == line_end)
        {
            printer::error(
                "unexpected end of line while parsing @supdef options",
                *std::prev(tok), origdata, &format
            );
            return std::nullopt;
        }
        if (tok->kind != token_kind::keyword || tok->keyword != keyword_kind::begin)
        {
            printer::error(
                "missing @supdef begin keyword",
                *tok, origdata, &format
            );
            return std::nullopt;
        }
        opts = registered_supdef::parse_options(optstring);
    }

    std::advance(tok, 1);
    skipped = skipws(tok, line_end);
    if (tok == line_end)
    {
        printer::error(
            "unexpected end of line while parsing @supdef name",
            *std::prev(tok), origdata, &format
        );
        return std::nullopt;
    }
    if (skipped == 0)
        printer::warning(
            "missing whitespace after @supdef begin keyword",
            *tok, origdata, &format
        );
    if (tok->kind != token_kind::identifier)
    {
        printer::error(
            "expected identifier after @supdef begin keyword",
            *tok, origdata, &format
        );
        return std::nullopt;
    }
    name = std::move(tok->data.value());

    std::advance(tok, 1);
    skipws(tok, line_end);
    if (tok != line_end)
    {
        printer::warning(
            "unexpected tokens after @supdef begin keyword",
            *tok, origdata, &format
        );
    }

    return std::make_pair(opts, name);
}

std::optional<
    std::tuple<
        ::supdef::parser::registered_runnable::lang,
        ::supdef::parser::registered_runnable::options,
        std::u32string
    >
> supdef::parser::parse_runnable_start(
    std::list<token>::const_iterator line_start,
    std::list<token>::const_iterator line_end,
    const std::u32string& origdata
)
{
    // TODO
    return std::nullopt;
}

bool ::supdef::parser::parse_supdef_runnable_end(
    std::list<token>::const_iterator line_start,
    std::list<token>::const_iterator line_end,
    const std::u32string& origdata
)
{
    auto tok = line_start;

    skipws(tok, line_end);
    if (tok == line_end || tok->kind != token_kind::at)
        return false;
    
    std::advance(tok, 1);
    skipws(tok, line_end);
    if (tok == line_end || tok->kind != token_kind::keyword || tok->keyword != keyword_kind::end)
        return false;

    std::advance(tok, 1);
    skipws(tok, line_end);
    if (tok != line_end)
        printer::warning(
            "unexpected tokens after @end keyword",
            *tok, origdata, &format
        );

    return true;
}

/*
 * supdefs format (optional spaces are omitted):
 *     ^@supdef <options>? begin <name>$
 *         <supdef-body>
 *     ^@end$
 *
 * runnable format (optional spaces are omitted):
 *     ^@runnable <language> <options>? begin <name>$
 *         <runnable-body>
 *     ^@end$
 */
// (for now, ignore runnables)
// retrieve supdefs and runnables
void ::supdef::parser::do_stage5()
{
    std::string errmsg;
    ::supdef::token errtok;
    const auto& orig_data = m_file.original_data();

    auto token = m_tokens.cbegin();
    while (token != m_tokens.cend())
    {
        auto line_start = token;
        auto line_end = std::find_if(
            token, m_tokens.cend(),
            [](const ::supdef::token& tok) {
                return tok.kind == token_kind::newline;
            }
        );
        if (line_end == m_tokens.cend())
            break; // can't have a one-line supdef or runnable

        auto supdef_start = parse_supdef_start(line_start, line_end, orig_data);
        if (supdef_start)
        {
            registered_supdef sd;
            sd.opts = std::move(std::get<0>(*supdef_start));
            sd.name = std::move(std::get<1>(*supdef_start));

            // remove line
            m_tokens.erase(line_start, line_end);
            token = line_start;

            // append lines of tokens while not end of supdef
            do
            {
                line_start = token;
                line_end = std::find_if(
                    token, m_tokens.cend(),
                    [](const ::supdef::token& tok) {
                        return tok.kind == token_kind::newline;
                    }
                );
                if (line_end == m_tokens.cend())
                {
                    errmsg = "unexpected end of file while parsing supdef";
                    errtok = *std::prev(token);
                    goto error;
                }

                if (parse_supdef_runnable_end(line_start, line_end, orig_data))
                    break;

                std::vector<::supdef::token> line;
                std::copy(line_start, line_end, std::back_inserter(line));
                sd.lines.push_back(std::move(line));
                m_tokens.erase(line_start, line_end);
                token = line_start;
            } while (token != m_tokens.cend());

            // remove end of supdef
            if (!parse_supdef_runnable_end(token, m_tokens.cend(), orig_data))
            {
                errmsg = "missing @end keyword after supdef";
                errtok = *std::prev(token);
                goto error;
            }
            m_tokens.erase(line_start, line_end);

            bool inserted;
            std::tie(std::ignore, inserted) = m_supdefs.emplace(sd.name, std::move(sd));
            if (!inserted)
                printer::warning(
                    "supdef `" + format(sd.name) + "` already defined",
                    *line_start, orig_data, &format
                );
        }
    }

    return;

error:
    printer::error(
        errmsg, errtok, orig_data, &format
    );
    return;

fatal:
    printer::fatal(
        errmsg, errtok, orig_data, &format
    );
    ::exit(EXIT_FAILURE);
}

[[__nodiscard__]]
bool ::supdef::parser::add_child_parser(const stdfs::path& filename, ::supdef::token_kind pathtype) noexcept
{
    bool inserted;

    if (filename.is_absolute() && stdfs::exists(filename))
    {
        std::tie(std::ignore, inserted) = m_imported_parsers.emplace(filename);
        if (!inserted)
            printer::warning(
                "file `" + filename.string() + "` already imported",
                *m_tokens.begin(), m_file.original_data(), &format
            );
        return true;
    }

    const auto& import_paths = ::supdef::globals::get_supdef_import_paths();
    
    if (pathtype == ::supdef::token_kind::string_literal)
    {
        const stdfs::path import_from = m_file.filename()->parent_path();
        if (stdfs::exists(import_from / filename))
        {
            std::tie(std::ignore, inserted) = m_imported_parsers.emplace(import_from / filename);
            if (!inserted)
                printer::warning(
                    "file `" + (import_from / filename).string() + "` already imported",
                    *m_tokens.begin(), m_file.original_data(), &format
                );
            return true;
        }
    }

    for (const auto& imppath : import_paths)
    {
        if (stdfs::exists(imppath / filename))
        {
            std::tie(std::ignore, inserted) = m_imported_parsers.emplace(imppath / filename);
            if (!inserted)
                printer::warning(
                    "file `" + (imppath / filename).string() + "` already imported",
                    *m_tokens.begin(), m_file.original_data(), &format
                );
            return true;
        }
    }

    return false;
}

namespace
{
    static constexpr auto enum_name_as_string(const ::supdef::token& e) noexcept -> std::string_view
    {
        using namespace std::string_view_literals;
        std::string_view ret = e.keyword.has_value() ?
            magic_enum::enum_name(e.keyword.value()) :
            "none"sv;
        return ret;
    };
    static constexpr auto fmt_data(const std::optional<std::u32string>& data) noexcept -> std::string
    {
        using namespace std::string_literals;
        return data.has_value()  ?
            format(data.value()) :
            "none"s;
    };
    static constexpr auto fmt_whitespace(const ::supdef::token& e) noexcept -> std::string
    {
        using namespace std::string_literals;
        assert(e.data.has_value() && e.data.value().size() == 1);
        auto echar = static_cast<uint32_t>(e.data.value().at(0));
        return e.kind == ::supdef::token_kind::newline      ?
            "<newline>: U+"s + std::format("{:04X}", echar) :
            "<horizontal whitespace>: U+"s + std::format("{:04X}", echar);
    };
    static constexpr auto fmt_token(const ::supdef::token& e) noexcept -> std::string
    {
        if (e.kind == ::supdef::token_kind::newline || e.kind == ::supdef::token_kind::horizontal_whitespace)
            return fmt_whitespace(e);
        std::string prefix = "",
                    suffix = "";
        if (e.kind == ::supdef::token_kind::char_literal)
            prefix = suffix = "'";
        if (e.kind == ::supdef::token_kind::string_literal)
            prefix = suffix = "\"";
        if (e.kind == ::supdef::token_kind::hex_integer_literal)
            prefix = "0x";
        if (e.kind == ::supdef::token_kind::octal_integer_literal)
            prefix = "0";
        if (e.kind == ::supdef::token_kind::binary_integer_literal)
            prefix = "0b";
        return prefix + fmt_data(e.data) + suffix;
    };
    static constexpr void output_token_to(
        std::ostream& os, const ::supdef::token& tok, ::supdef::parser::output_kind kind, size_t token_index = -1
    )
    {
        switch (kind)
        {
        case ::supdef::parser::output_kind::text:
            switch (tok.kind)
            {
            // literals
            case ::supdef::token_kind::char_literal:
                [[__fallthrough__]];
            case ::supdef::token_kind::string_literal:
                [[__fallthrough__]];
            case ::supdef::token_kind::hex_integer_literal:
                [[__fallthrough__]];
            case ::supdef::token_kind::octal_integer_literal:
                [[__fallthrough__]];
            case ::supdef::token_kind::binary_integer_literal:
                os << fmt_token(tok);
                break;

            // EOF
            case ::supdef::token_kind::eof:
                break;

            // everything else
            default:
                os << format(tok.data.value());
                break;
            }
            break;
        case ::supdef::parser::output_kind::tokens:
            os << "<token n°" << token_index << ">\n"
               << "  kind:    " << magic_enum::enum_name(tok.kind) << '\n'
               << "  keyword: " << enum_name_as_string(tok) << '\n'
               << "  data:    " << fmt_token(tok) << '\n';
            break;
        case ::supdef::parser::output_kind::ast:
            // TODO: implement
            break;
        default: break;
        }
    }
}

void ::supdef::parser::output_to(std::ostream& os, output_kind kind)
{
    if (kind & output_kind::text)
    {
        for (auto&& [i, token] : std::views::enumerate(m_tokens))
            output_token_to(os, token, output_kind::text, i);
    }
    if (kind & output_kind::tokens)
    {
        for (auto&& [i, token] : std::views::enumerate(m_tokens))
            output_token_to(os, token, output_kind::tokens, i);
    }
    if (kind & output_kind::ast)
    {
        // TODO: implement
    }
    if (kind & output_kind::imports)
    {
        const auto& processed = ::supdef::globals::get_already_processed_files();
        os << "imported files:\n";
        if (processed.size() > 1)
        {
            for (auto&& file : processed)
            {
                if (file != m_file.filename())
                    os << "  " << file->string() << '\n';
            }
        }
        else
            os << "  none\n";
    }
    if (kind & output_kind::original)
        os << format(m_file.original_data());
}

void ::supdef::parser::output_to(const stdfs::path& filename, output_kind kind)
{
    std::ofstream ofs(filename);
    this->output_to(ofs, kind);
}
