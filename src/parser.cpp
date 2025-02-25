#include <file.hpp>
#include <unicode.hpp>
#include <parser.hpp>
#include <tokenizer.hpp>
#include <detail/globals.hpp>

GLOBAL_GETTER_DECL(
    std::vector<shared_ptr<const stdfs::path>>,
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
GLOBAL_XXHASHER_DECL(supdef::parser::registered_runnable::lang::identifier, 64, runnable_language_identifier_enum);

supdef::parser::parser(const stdfs::path& filename)
    : m_ctx()
    , m_file(stdfs::canonical(filename))
    , m_tokens()
    , m_imported_parsers()
    , m_supdefs(32, ::supdef::globals::get_xxhasher<supdef_map_type::key_type, 64UL>())
    , m_runnables(32, ::supdef::globals::get_xxhasher<runnable_map_type::key_type, 64UL>())
{
    ::supdef::globals::get_already_processed_files().push_back(m_file.filename());
}

supdef::parser::parser(stdfs::path&& filename)
    : m_ctx()
    , m_file(stdfs::canonical(std::move(filename)))
    , m_tokens()
    , m_imported_parsers()
    , m_supdefs(32, ::supdef::globals::get_xxhasher<supdef_map_type::key_type, 64UL>())
    , m_runnables(32, ::supdef::globals::get_xxhasher<runnable_map_type::key_type, 64UL>())
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
        stdranges::advance(token, 1);
    }
}

// remove comments
void ::supdef::parser::do_stage3()
{
    for (auto token = m_tokens.begin(); token != m_tokens.end(); stdranges::advance(token, 1))
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
        stdranges::advance(cpy, -1);
        while (cpy != begin && cpy->kind != ::supdef::token_kind::newline)
        {
            if (cpy->kind != ::supdef::token_kind::horizontal_whitespace)
                return false;
            stdranges::advance(cpy, -1);
        }
        return true;
    }

    static auto reverse_find(
        const std::bidirectional_iterator auto begin, const std::bidirectional_iterator auto end,
        auto&& value
    )
    {
        auto iter = end;
        while (iter != begin)
        {
            stdranges::advance(iter, -1);
            if (*iter == value)
                return iter;
        }
        return iter;
    }

    static auto reverse_find_if(
        const std::bidirectional_iterator auto begin, const std::bidirectional_iterator auto end,
        auto&& pred
    )
    {
        auto iter = end;
        while (iter != begin)
        {
            stdranges::advance(iter, -1);
            if (pred(*iter))
                return iter;
        }
        return iter;
    }

    static auto replace_from_to(
        auto& destcont, std::input_iterator auto destfirst, std::input_iterator auto destlast,
        std::input_iterator auto srcfirst, std::input_iterator auto srclast
    )
    {
        destcont.erase(destfirst, destlast);
        return stdranges::copy(srcfirst, srclast, std::inserter(destcont, destlast));
    }

    static auto replace_from_to(
        auto& destcont, std::input_iterator auto destfirst, std::input_iterator auto destlast,
        stdranges::input_range auto&& srcrange
    )
    {
        destcont.erase(destfirst, destlast);
        return stdranges::copy(
            std::forward<std::remove_reference_t<decltype(srcrange)>>(srcrange),
            std::inserter(destcont, destlast)
        );
    }

    static auto replace_from_to(
        auto& destcont, std::input_iterator auto destfirst, std::input_iterator auto destlast,
        auto&& srcelem
    )
    {
        destcont.erase(destfirst, destlast);
        return destcont.insert(
            destlast,
            std::forward<std::remove_reference_t<decltype(srcelem)>>(srcelem)
        );
    }

    template <typename T>
    static auto replace_from_to(
        std::list<T>& destcont, std::input_iterator auto destfirst, std::input_iterator auto destlast,
        std::list<T>&& srcrange
    )
    {
        destcont.erase(destfirst, destlast);
        destcont.splice(destlast, std::move(srcrange));
    }

    static size_t skipws(points_to_token_and_input auto& iter, const points_to_token_and_input auto end, bool skip_newlines = false)
    {
        size_t count = 0;
        while (iter != end &&
                (iter->kind == ::supdef::token_kind::horizontal_whitespace ||
                    (skip_newlines && iter->kind == ::supdef::token_kind::newline)))
        {
            stdranges::advance(iter, 1);
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
                stdranges::advance(iter, 1);
                count++;
            }
        }
        else
        {
            while (iter != end && !pred(*iter))
            {
                stdranges::advance(iter, 1);
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
                stdranges::advance(iter, 1);
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
                stdranges::advance(iter, 1);
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

    for (auto token = m_tokens.cbegin(); token != m_tokens.cend(); stdranges::advance(token, 1))
    {
        if (token->kind != token_kind::at)
            continue;
        if (!at_start_of_line(token, m_tokens.cbegin()))
            continue;

        const auto import_start = token;
        std::remove_const_t<decltype(import_start)> import_end;

        stdranges::advance(token, 1);
        skipws(token, m_tokens.cend());
        if (token->kind != token_kind::keyword)
            continue;
        if (token->keyword != keyword_kind::import)
            continue;

        stdranges::advance(token, 1);
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
            
            stdranges::advance(token, 1);
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
    for (const parser& p : m_imported_parsers)
    {
        // operator<=>(const parser& rhs) compares source file paths,
        // so we can const_cast because none of the stages modify the
        // source file path, and no ordering is violated
        const_cast<parser&>(p).do_stage1();
        const_cast<parser&>(p).do_stage2();
        const_cast<parser&>(p).do_stage3();
        const_cast<parser&>(p).do_stage4();
        const_cast<parser&>(p).do_stage5();
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

    stdranges::advance(tok, 1);
    skipws(tok, line_end);
    if (tok == line_end || tok->kind != token_kind::keyword || tok->keyword != keyword_kind::supdef)
        return std::nullopt;

    stdranges::advance(tok, 1);
    skipped = skipws(tok, line_end);
    if (tok == line_end)
        return std::nullopt;
    if (skipped == 0)
        printer::warning(
            "missing whitespace after @supdef keyword",
            *tok, origdata, &format
        );
    
    if (tok->kind == token_kind::keyword && tok->keyword == keyword_kind::begin)
        opts = registered_supdef::options{ .eat_newlines = true };
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

    stdranges::advance(tok, 1);
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

    stdranges::advance(tok, 1);
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
    
    stdranges::advance(tok, 1);
    skipws(tok, line_end);
    if (tok == line_end || tok->kind != token_kind::keyword || tok->keyword != keyword_kind::end)
        return false;

    stdranges::advance(tok, 1);
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
        // skip empty lines
        if (token->kind == token_kind::newline)
        {
            stdranges::advance(token, 1);
            continue;
        }

        registered_supdef sd;
        auto move_line = [&sd, this](auto iter_start, auto iter_end) {
            using line_type = decltype(std::declval<registered_supdef>().lines)::value_type;
            line_type line;
            std::copy(iter_start, iter_end, std::back_inserter(line));
            sd.lines.push_back(std::move(line));
            this->m_tokens.erase(iter_start, iter_end);
        };

        auto line_start = token;
        auto line_end = std::find_if(
            line_start, m_tokens.cend(),
            [](const ::supdef::token& tok) {
                return tok.kind == token_kind::newline;
            }
        );

        // can't have a one-line supdef or runnable
        // TODO: issue a warning if it matches the supdef or runnable start syntax
        if (line_end == m_tokens.cend())
            break;

        auto supdef_start = parse_supdef_start(line_start, line_end, orig_data);
        if (supdef_start)
        {
            sd.opts = std::move(std::get<0>(*supdef_start));
            sd.name = std::move(std::get<1>(*supdef_start));

            // keep the newline by taking the next as the real start
            // of the supdef body, without erasing it
            token = std::next(line_end);

            // remove line
            m_tokens.erase(line_start, line_end);

            // append lines of tokens while not end of supdef
            bool got_sd_end = false;
            do
            {
                // again, skip empty lines
                if (token->kind == token_kind::newline)
                {
                    stdranges::advance(token, 1);
                    continue;
                }

                line_start = token;
                line_end = std::find_if(
                    line_start, m_tokens.cend(),
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
                {
                    got_sd_end = true;
                    break;
                }

                // go after the newline (i.e. the next line)
                token = std::next(line_end);

                // transfer line to supdef as a body line
                move_line(line_start, line_end);

                // this time, erase the newline
                m_tokens.erase(line_end);
            } while (token != m_tokens.cend());

            if (!got_sd_end)
            {
                errmsg = "missing @end keyword after supdef";
                errtok = *std::prev(token);
                goto error;
            }

            // go to next line
            token = std::next(line_end);

            // remove @end of supdef
            m_tokens.erase(line_start, line_end);

            bool inserted;
            std::tie(std::ignore, inserted) = m_supdefs.emplace(sd.name, std::move(sd));
            if (!inserted)
                printer::warning(
                    "supdef `" + format(sd.name) + "` already defined",
                    *line_start, orig_data, &format
                );

            continue;
        }

        auto runnable_start = parse_runnable_start(line_start, line_end, orig_data);
        if (runnable_start)
        {
            // TODO
            continue;
        }

        // skip line
        token = std::next(line_end);
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

using __pair_t = std::pair<
    const ::supdef::parser::registered_runnable::lang::identifier,
    ::supdef::parser::registered_runnable::lang::execinfo
>;
#undef  __ID
#undef  __INFO
#define __ID(x) ::supdef::parser::registered_runnable::lang::identifier::x
#define __INFO(...) ::supdef::parser::registered_runnable::lang::execinfo{__VA_ARGS__}

#undef  __SRC
#undef  __EXE
#undef  __COMP
#undef  __INTERP
#undef  __VERS
#undef  __JOIN
#define __SRC ::supdef::parser::registered_runnable::lang::cmdline_source_placeholder {}
#define __EXE ::supdef::parser::registered_runnable::lang::cmdline_executable_placeholder {}
#define __COMP ::supdef::parser::registered_runnable::lang::cmdline_compiler_placeholder {}
#define __INTERP ::supdef::parser::registered_runnable::lang::cmdline_interpreter_placeholder {}
#define __VERS ::supdef::parser::registered_runnable::lang::cmdline_version_placeholder {}
#define __JOIN(a, b) a, ::supdef::parser::registered_runnable::lang::cmdline_joinargs_placeholder {}, b

namespace
{
    static std::optional<stdfs::path> __which(const std::string& cmd)
    {
        namespace bp = boost::process;
        auto found = bp::search_path(cmd);
        if (found.empty())
            return std::nullopt;
        return found.c_str();
    }


    static const ::supdef::umap<
        ::supdef::parser::registered_runnable::lang::identifier,
        ::supdef::parser::registered_runnable::lang::execinfo
    > DEFAULT_LANG_INFO(void)
    {
        return ::supdef::umap<
            ::supdef::parser::registered_runnable::lang::identifier,
            ::supdef::parser::registered_runnable::lang::execinfo
        >{
            {
                __pair_t{
                    __ID(c), __INFO(
                        .version = U"gnu23",
                        .compiler = __which("gcc"),
                        .interpreter = std::nullopt,
                        .compiler_cmdline = {
                            __COMP, U"-xc", __JOIN(U"-std=", __VERS), U"-Wall", U"-Wextra", __SRC, U"-o", __EXE
                        },
                        .execution_cmdline = {
                            __EXE
                        }
                    )
                },
                __pair_t{
                    __ID(cpp), __INFO(
                        .version = U"gnu++23",
                        .compiler = __which("g++"),
                        .interpreter = std::nullopt,
                        .compiler_cmdline = {
                            __COMP, U"-xc++", __JOIN(U"-std=", __VERS), U"-Wall", U"-Wextra", __SRC, U"-o", __EXE
                        },
                        .execution_cmdline = {
                            __EXE
                        }
                    )
                },
                __pair_t{
                    __ID(rust), __INFO(
                        .version = U"2021",
                        .compiler = __which("rustc"),
                        .interpreter = std::nullopt,
                        .compiler_cmdline = {
                            __COMP, __JOIN(U"--edition=", __VERS), __SRC, U"-o", __EXE
                        },
                        .execution_cmdline = {
                            __EXE
                        }
                    )
                },
                __pair_t{
                    __ID(d), __INFO(
                        .version = U"",
                        .compiler = __which("dmd"),
                        .interpreter = std::nullopt,
                        .compiler_cmdline = {
                            __COMP, __SRC, U"-of", __EXE
                        },
                        .execution_cmdline = {
                            __EXE
                        }
                    )
                },
                __pair_t{
                    __ID(zig), __INFO(
                        .version = U"",
                        .compiler = __which("zig"),
                        .interpreter = std::nullopt,
                        .compiler_cmdline = {
                            __COMP, __SRC, U"-o", __EXE
                        },
                        .execution_cmdline = {
                            __EXE
                        }
                    )
                },
            #if 0 // ignore c# and f# for now
                __pair_t{
                    __ID(csharp), __INFO(
                        .version = U"",
                        .compiler = __which("csc"),
                        .interpreter = __which("mono"),
                        .compiler_cmdline = {
                            __COMP, __SRC, U"-out:" __EXE
                        },
                        .execution_cmdline = {
                            __EXE
                        }
                    )
                },
                __pair_t{
                    __ID(fsharp), __INFO(
                        .version = U"",
                        .compiler = __which("fsc"),
                        .interpreter = __which("mono"),
                        .compiler_cmdline = {
                            __COMP, __SRC, U"-o", __EXE
                        },
                        .execution_cmdline = {
                            __EXE
                        }
                    )
                },
            #endif
                __pair_t{
                    __ID(java), __INFO(
                        .version = U"",
                        .compiler = __which("javac"),
                        .interpreter = __which("java"),
                        .compiler_cmdline = {
                            __COMP, __SRC
                        },
                        .execution_cmdline = {
                            __INTERP, __EXE
                        }
                    )
                },
                __pair_t{
                    __ID(ocaml), __INFO(
                        .version = U"",
                        .compiler = __which("ocamlc"),
                        .interpreter = __which("ocaml"),
                        .compiler_cmdline = {
                            __COMP, __SRC, U"-o", __EXE
                        },
                        .execution_cmdline = {
                            __EXE
                        }
                    )
                },
                __pair_t{
                    __ID(racket), __INFO(
                        .version = U"",
                        .compiler = std::nullopt,
                        .interpreter = __which("racket"),
                        .compiler_cmdline = { },
                        .execution_cmdline = {
                            __INTERP, __SRC
                        }
                    )
                },
                __pair_t{
                    __ID(haskell), __INFO(
                        .version = U"",
                        .compiler = __which("ghc"),
                        .interpreter = std::nullopt,
                        .compiler_cmdline = {
                            __COMP, __SRC, U"-o", __EXE
                        },
                        .execution_cmdline = {
                            __EXE
                        }
                    )
                },
                __pair_t{
                    __ID(python), __INFO(
                        .version = U"3.12",
                        .compiler = std::nullopt,
                        .interpreter = __which("python"),
                        .compiler_cmdline = { },
                        .execution_cmdline = {
                            __JOIN(__INTERP, __VERS), __SRC
                        }
                    )
                },
                __pair_t{
                    __ID(shell), __INFO(
                        .version = U"",
                        .compiler = std::nullopt,
                        .interpreter = __which("sh"),
                        .compiler_cmdline = { },
                        .execution_cmdline = {
                            __INTERP, __SRC
                        }
                    )
                },
                __pair_t{
                    __ID(perl), __INFO(
                        .version = U"",
                        .compiler = std::nullopt,
                        .interpreter = __which("perl"),
                        .compiler_cmdline = { },
                        .execution_cmdline = {
                            __INTERP, __SRC
                        }
                    )
                },
                __pair_t{
                    __ID(ruby), __INFO(
                        .version = U"",
                        .compiler = std::nullopt,
                        .interpreter = __which("ruby"),
                        .compiler_cmdline = { },
                        .execution_cmdline = {
                            __INTERP, __SRC
                        }
                    )
                }
            },
            0, ::supdef::globals::get_xxhasher<::supdef::parser::registered_runnable::lang::identifier, 64UL>()
        };
    }
}

#undef  __ID
#undef  __INFO
#undef  __SRC
#undef  __EXE
#undef  __COMP
#undef  __INTERP
#undef  __VERS
#undef  __JOIN

void ::supdef::parser::do_stage6()
{
    namespace bd = ::boost::debug;
    if (bd::under_debugger())
        bd::debugger_break();
    m_ctx.make_empty();
    m_ctx.push(
        ::supdef::parser::substitution_context{
            .variables = {},
            .default_langinfo = DEFAULT_LANG_INFO(),
            .default_sdopts = std::nullopt,
            .default_runopts = std::nullopt,
            .supdefs = &m_supdefs,
            .runnables = &m_runnables,
            .arguments = {},
            .toplevel = true,
            .in_supdef = false,
            .in_runnable = false
        }
    );

    this->execute_toplevel();
}

void ::supdef::parser::execute_toplevel()
{
    auto tok = m_tokens.begin();
    while (tok != m_tokens.end())
    {
        /*
         * we have 4 things to look for:
         * 1) supdef/runnable calls :
         *     <macro-name> ( '<' <option-list> '>' )? '(' <arg-list> ')'
         * 2) builtin function calls :
         *     '@' <function-name> '(' <arg-list> ')'
         * 3) variable assignments :
         *     '@' 'set' <variable-name> '=' <value>
         * 4) pragmas :
         *     '@' 'pragma' <pragma-name> <pragma-args>
         * 
         * actions to take:
         * 1) supdef/runnable calls :
         *    - add a substitution context to the stack
         *    - call the supdef/runnable replacer with the substitution context,
         *        and the arguments (potentially substituting macros in the arguments, first)
         * 2) builtin function calls :
         *    - same as above but with the builtin function replacer
         * 3) variable assignments :
         *    - update the variable in the substitution context
         * 4) pragmas :
         *    - update substitution context accordingly
         */

        const auto tokcpy = tok;
        auto rescanfromtok = [tokcpy, this] {
            if (tokcpy != m_tokens.begin())
                return std::prev(tokcpy);
            return m_tokens.begin();
        };

        switch (tok->kind)
        {
        case token_kind::at:
            tok = this->execute_directive(tok, tokcpy);
            continue;
        case token_kind::identifier: {
        } break;
        case token_kind::dollar:
            tok = this->execute_variable_substitution(tok, tokcpy);
            continue;
        default:
            break;
        }

        stdranges::advance(token, 1);
    }
}

namespace
{
    std::list<::supdef::token> isolate_line(
        std::list<::supdef::token>& tokens,
        std::list<::supdef::token>::iterator& pos
    ) noexcept
    {
        std::list<::supdef::token> line;
        auto line_start = reverse_find_if(
            pos, tokens.begin(),
            [](const ::supdef::token& tok) {
                return tok.kind == ::supdef::token_kind::newline;
            }
        );
        auto line_end = stdranges::find_if(
            pos, tokens.end(),
            [](const ::supdef::token& tok) {
                return tok.kind == ::supdef::token_kind::newline;
            }
        );
        line.splice(line.begin(), tokens, line_start, line_end);
        pos = line_end;
        return line;
    }
}

std::list<::supdef::token>::iterator supdef::parser::execute_directive(
    std::list<::supdef::token>::iterator tok, const std::list<::supdef::token>::iterator tokcpy
)
{
    auto rescanfromtok = [tokcpy, this] {
        if (tokcpy != m_tokens.begin())
            return std::prev(tokcpy);
        return m_tokens.begin();
    };
    auto ret = tokcpy;
    stdranges::advance(tok, 1);
    skipws(tok, m_tokens.cend());
    if (tok == m_tokens.end())
        return m_tokens.end();
    if (tok->kind != token_kind::keyword)
    {
        printer::warning(
            "expected keyword after @",
            *tok, m_file.original_data(), &format
        );
        return tok;
    }
    switch (tok->keyword)
    {
    // pragmas
    case keyword_kind::pragma: {
        // TODO: shall we delete the whole line from here ?
        // Or maybe from execute_toplevel() ?
        // Or maybe even splice the line out of m_tokens before passing iterators to execute_pragma() ?
        bool at_start = at_start_of_line(tok, m_tokens.cbegin());
        if (!at_start)
        {
            printer::warning(
                "pragma must be at the start of a line",
                *tok, m_file.original_data(), &format
            );
            return tok;
        }
        std::list line = this->m_ctx.top().toplevel ? isolate_line(m_tokens, tok) : copy_line(m_tokens, tok);
        // NOTE: tok is correctly updated by isolate_line()
        this->execute_pragma(line.begin(), line.end());
    } break;
    // these are handled in the next stage
    case keyword_kind::dump:
        [[__fallthrough__]];
    case keyword_kind::embed:
        break;
    }
}

void supdef::parser::execute_pragma(
    const std::list<::supdef::token>::iterator start, const std::list<::supdef::token>::iterator end
)
{
    // @pragma supdef <pragma-name>
    std::u32string_view supdef_pragmas[] = {
        U"newline"
    };
    // @pragma runnable <pragma-name>
    std::u32string_view runnable_pragmas[] = {
        U"newline", U"mode" 
    };
    // @pragma runnable <lang-identifier> <pragma-name>
    std::u32string_view runnable_lang_pragmas[] = {
        U"version", U"compiler_path", U"interpreter_path",
        U"compiler_cmdline", U"interpreter_cmdline"
    };
    // @pragma <pragma-name>
    std::u32string_view other_pragmas[] = {
        
    };

    auto iter = start;
    stdranges::advance(iter, 1);
    if (iter == end)
        return;
    if (iter->kind != token_kind::identifier && iter->kind != token_kind::keyword)
    {
        printer::warning(
            "expected identifier or keyword after @pragma",
            *iter, m_file.original_data(), &format
        );
        return;
    }

    std::u32string_view pragma_name = iter->data.value();
    if (strmatch(pragma_name, U"supdef") && stdranges::size(supdef_pragmas))
    {
        stdranges::advance(iter, 1);
        skipws(iter, end);
        if (iter == end)
            return;
        if (iter->kind != token_kind::identifier)
        {
            printer::warning(
                "expected identifier after @pragma supdef",
                *iter, m_file.original_data(), &format
            );
            return;
        }
        pragma_name = iter->data.value();
        
        constexpr size_t possibilities = stdranges::distance(stdranges::begin(supdef_pragmas), stdranges::end(supdef_pragmas));
        std::bitset<possibilities> pragma_macthes;
        stdranges::transform(
            supdef_pragmas, bitset_inserter{pragma_macthes},
            [pragma_name](const std::u32string_view& sv) {
                return strmatch(pragma_name, sv, false);
            }
        );

        if (pragma_macthes.none())
        {
            printer::error(
                "unknown pragma `@pragma supdef " + format(pragma_name) + "` (skiping)",
                *iter, m_file.original_data(), &format
            );
            return;
        }
        size_t longest     = 0,
               longest_idx = 0;
        for (size_t i = 0; i < possibilities; i++)
        {
            if (pragma_macthes[i] && supdef_pragmas[i].size() > longest)
            {
                longest = supdef_pragmas[i].size();
                longest_idx = i;
            }
        }
        switch (longest_idx)
        {
        case 0: { // newline
            stdranges::advance(iter, 1);
            skipws(iter, end);
            if (iter == end)
            {
                printer::warning(
                    "expected value after @pragma supdef newline",
                    *std::prev(iter), m_file.original_data(), &format
                );
                return;
            }
            auto astribool = supdef::registered_base::parse_bool_val(iter->data.value());
            if (boost::logic::indeterminate(astribool))
            {
                printer::warning(
                    "expected boolean value after @pragma supdef newline",
                    *iter, m_file.original_data(), &format
                );
                return;
            }
            auto& top_sdopts = m_ctx.top().default_sdopts;
            top_sdopts = std::optional(
                std::move(top_sdopts).value_or(supdef::parser::registered_supdef::none_options)
            ).transform(
                [astribool](auto&& opts) {
                    opts.eat_newlines = static_cast<bool>(astribool) ? 0U : 1U;
                    return opts;
                }
            );
        } break;
        default:
            std::unreachable();
        }

        return;
    }
    if (strmatch(pragma_name, U"runnable") && stdranges::size(runnable_pragmas) + stdranges::size(runnable_lang_pragmas))
    {
        TODO("implement runnable pragma parsing");
        return;
        stdranges::advance(iter, 1);
        skipws(iter, end);
        if (iter == end)
            return;
        if (iter->kind != token_kind::identifier)
        {
            printer::warning(
                "expected identifier after @pragma runnable",
                *iter, m_file.original_data(), &format
            );
            return;
        }

        std::u32string_view lang_or_prag = iter->data.value();
        if (supdef::parser::registered_runnable::is_lang_identifier(lang_or_prag))
        {
            // parse runnable lang pragma
            auto lang = supdef::parser::registered_runnable::parse_lang(lang_or_prag);
        }
        else
        {
            // parse runnable pragma
        }

        return;
    }
    TODO("implement other pragma parsing");
    return;
}

std::list<::supdef::token>::iterator supdef::parser::execute_variable_substitution(
    std::list<::supdef::token>::iterator tok, const std::list<::supdef::token>::iterator tokcpy
)
{
    auto rescanfromtok = [tokcpy, this] {
        if (tokcpy != m_tokens.begin())
            return std::prev(tokcpy);
        return m_tokens.begin();
    };
    auto ret = tokcpy;
    stdranges::advance(tok, 1);
    if (tok == m_tokens.end())
        return m_tokens.end();
    if (tok->kind == token_kind::dollar)
    {
        // escape sequence
        ret = m_tokens.erase(tok);
        return ret;
    }
    if (tok->kind == token_kind::identifier)
    {
        // variable substitution
        std::list<token> subst;
        auto&& [traversed, gotit] = m_ctx.traverse_until(
            [&subst, &tok, this](const ::supdef::parser::substitution_context& ctx) noexcept -> bool {
                auto var = ctx.variables.find(tok->data.value());
                if (var != ctx.variables.end())
                {
                    subst = var->second;
                    return true;
                }
                return false;
            }
        );
        if (!gotit)
        {
            printer::warning(
                "undefined variable `" + format(tok->data.value()) + "`",
                *tok, m_file.original_data(), &format
            );
            return tok;
        }
        replace_from_to(m_tokens, tokcpy, std::next(tok), std::move(subst));
        return rescanfromtok();
    }
    if (!m_ctx.top().toplevel)
    {
        if (tok->kind == token_kind::integer_literal)
        {
            // argument substitution
            std::list<token> subst;
            unsigned argnum;
            try
            {
                argnum = std::stoul(tok->data.value());
            }
            catch (const std::exception& e)
            {
                printer::fatal(
                    "invalid argument number `" + format(tok->data.value()) + "`" +
                    " (got exception: " + e.what() + ")",
                    *tok, m_file.original_data(), &format
                );
                ::exit(EXIT_FAILURE);
            }
            if (argnum >= m_ctx.top().arguments.size())
            {
                printer::warning(
                    "argument number `" + format(tok->data.value()) + "` out of range",
                    *tok, m_file.original_data(), &format
                );
                // erase the from `tokcpy` to `std::next(tok)`
                return m_tokens.erase(tokcpy, std::next(tok));
            }
            subst = *stdranges::next(stdranges::begin(m_ctx.top().arguments), argnum);
            replace_from_to(m_tokens, tokcpy, std::next(tok), std::move(subst));
            return rescanfromtok();
        }
        if (tok->kind == token_kind::asterisk || tok->kind == token_kind::at)
        {
            // argument list substitution
            token_loc tokloc = *tok;
            tokloc.toksize = 1;
            const token comma_token{
                .loc = tokloc,
                .data = std::u32string{ U"," },
                .keyword = std::nullopt,
                .kind = token_kind::comma
            };
            const token space_token{
                .loc = tokloc,
                .data = std::u32string{ U" " },
                .keyword = std::nullopt,
                .kind = token_kind::horizontal_whitespace
            };
            std::list<token> subst;
            auto adaptor = stdviews::as_const | (
                tok->kind == token_kind::at ?
                    stdviews::join_with(comma_token) :
                    stdviews::join_with(space_token)
            );
            for (const auto& arg : m_ctx.top().arguments | adaptor)
                subst.push_back(arg);
            replace_from_to(m_tokens, tokcpy, std::next(tok), std::move(subst));
            return rescanfromtok();
        }
        if (tok->kind == token_kind::hash)
        {
            // argument count substitution
            token replacement{
                .loc = *tok,
                .data = std::u32string{ },
                .keyword = std::nullopt,
                .kind = token_kind::integer_literal
            };
            std::string argcount = std::to_string(m_ctx.top().arguments.size());
            ascii_to_char32_view argcount_iter{ argcount };
            replacement.data = std::u32string{ argcount_iter.begin(), argcount_iter.end() };
            replacement.loc.toksize = replacement.data.size();
            replace_from_to(m_tokens, tokcpy, std::next(tok), replacement);
            return rescanfromtok();
        }
    }
    return tok;
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
        std::ostream& os, const ::supdef::token& tok,
        ::supdef::parser::output_kind kind,
        size_t token_index = -1, std::string_view indent = ""
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
                os << indent << fmt_token(tok);
                break;

            // EOF
            case ::supdef::token_kind::eof:
                break;

            // everything else
            default:
                os << indent << format(tok.data.value());
                break;
            }
            break;
        case ::supdef::parser::output_kind::tokens:
            os << indent << "<token n°" << token_index << ">\n"
               << indent << "  kind:    " << magic_enum::enum_name(tok.kind) << '\n'
               << indent << "  keyword: " << enum_name_as_string(tok) << '\n'
               << indent << "  data:    " << fmt_token(tok) << '\n';
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
    if (kind & output_kind::supdefs)
    {
        size_t printed;
        os << "supdefs:\n";
        if (kind & recursive)
        {
            for (auto& p : m_imported_parsers)
            {
                printed = 0;
                os << "  in file " << p.m_file.filename()->string() << '\n';
                for (auto&& [name, supdef] : p.m_supdefs)
                {
                    os << "    " << format(name) << '\n';
                    printed++;
                    for (auto&& line : supdef.lines)
                    {
                        os << "      ";
                        for (auto&& token : line)
                            output_token_to(os, token, output_kind::text);
                        os << '\n';
                    }
                }
                if (!printed)
                    os << "    none\n";
            }
        }
        printed = 0;
        os << "  in file " << m_file.filename()->string() << '\n';
        for (auto&& [name, supdef] : m_supdefs)
        {
            os << "    " << format(name) << '\n';
            printed++;
            for (auto&& line : supdef.lines)
            {
                os << "      ";
                for (auto&& token : line)
                    output_token_to(os, token, output_kind::text);
                os << '\n';
            }
        }
        if (!printed)
            os << "    none\n";
    }
    if (kind & output_kind::original)
        os << format(m_file.original_data());
}

void ::supdef::parser::output_to(const stdfs::path& filename, output_kind kind)
{
    std::ofstream ofs(filename);
    this->output_to(ofs, kind);
}

/* #include <boost/test/impl/debug.ipp> */