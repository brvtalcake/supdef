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
    BOOST_IDENTITY_TYPE((::supdef::detail::xxhash<std::u32string, 64>)),
    u32string_xxhasher
);

supdef::parser::parser(const stdfs::path& filename)
    : m_file(stdfs::canonical(filename)), m_tokens(), m_imported_parsers(), m_supdefs(32, ::supdef::globals::get_u32string_xxhasher())
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

// process imports
void ::supdef::parser::do_stage4()
{
    for (auto token = m_tokens.cbegin(); token != m_tokens.cend(); std::advance(token, 1))
    {
        if (token->kind != token_kind::keyword)
            continue;
        if (token->keyword != keyword::import_)
            continue;
        auto next_token = std::next(token);
        while (next_token != m_tokens.cend() && next_token->kind == token_kind::horizontal_whitespace)
            std::advance(next_token, 1);
        if (next_token == m_tokens.cend())
        {
            printer::error(
                "unexpected end of file while parsing import statement",
                *token, m_file.original_data(), &format
            );
            return;
        }
        stdfs::path extracted_path;
        switch (next_token->kind)
        {
        case token_kind::string_literal:
            extracted_path = stdfs::path(next_token->data.value());
            break;
        case token_kind::langle: {

        } break;
        default:
            printer::error(
                "unexpected token while parsing import statement",
                *next_token, m_file.original_data(), &format
            );
            return;
        }
    }
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
    if (kind & output_kind::original)
        os << format(m_file.original_data());
}

void ::supdef::parser::output_to(const stdfs::path& filename, output_kind kind)
{
    std::ofstream ofs(filename);
    this->output_to(ofs, kind);
}

#if 0
static void splice_lines(std::u32string& str)
{
    using namespace std::string_literals;
    static const auto bs_string = U"\\\n"s;
    static const auto bs_replacement = U" "s;
    std::vector<size_t> found;
    while (true)
    {
        auto pos = str.find(bs_string, found.empty() ? 0 : found.back() + bs_string.length());
        if (pos == std::u32string::npos)
            break;
        found.push_back(pos);
    }
    std::sort(found.begin(), found.end(), std::less<size_t>());
    for (auto&& it = found.begin(); it != found.end(); ++it)
    {
        str.replace(*it, bs_string.length(), bs_replacement);
        // decrement all the other positions
        std::for_each(it + 1, found.end(), [](auto& x) { --x; });
    }
}

/*
 * Splice lines, as in the second stage of the C/C++ compilation process.
 */
void supdef::parser::do_stage1()
{
    auto& data = m_file.data();
    auto& orig = m_file.original_data();
    auto filename = m_file.filename();

    splice_lines(data);

    auto last_char = data.back();
    if (last_char != U'\n')
    {
        printer::warning(
            filename.get()->string() + " does not end with a newline character"
        );
        data.push_back(U'\n');
    }
}

void supdef::parser::do_stage2()
{
    auto& data = m_file.data();
    auto& orig = m_file.original_data();
    auto filename = m_file.filename();

    size_t errcount = 0;

    for (size_t i = 0; i < data.size(); ++i)
    {
        const char32_t& c = data.at(i);
        switch (c)
        {
        case U'/': {
            // replace comments with one space
            try
            {
                switch (data.at(i + 1))
                {
                case U'/': {
                    size_t j = i + 2;
                    while (data.at(j) != U'\n')
                        ++j;
                    data.replace(i, j - i, U" ");
                } break;
                case U'*': {
                    size_t j = i + 2;
                    while (!(data.at(j) == U'*' && data.at(j + 1) == U'/'))
                        ++j;
                    data.replace(i, j - i + 2, U" ");
                } break;
                default:
                    break;
                }
            }
            catch (const std::out_of_range& e)
            {
                printer::error(
                    "unexpected end of file while parsing comments"
                );
                errcount++;
                goto end;
            }
            catch (const std::exception& e)
            {
                using namespace std::string_literals;
                printer::fatal(
                    "unexpected error while parsing comments: "s + e.what()
                );
                ::exit(EXIT_FAILURE);
            }
        } break;
        case U'\\': {
            printer::warning(
                "backslash character outside of string or character literal"
            );
            continue;
        } break;
        case U'"': {
            try
            {
                ++i;
                while (data.at(i) != U'"')
                {
                    if (data.at(i) == U'\\')
                        i += 2;
                    else
                        ++i;
                }
                ++i;
            }
            catch (const std::out_of_range& e)
            {
                printer::error(
                    "unexpected end of file while parsing string literals"
                );
                errcount++;
                goto end;
            }
            catch (const std::exception& e)
            {
                using namespace std::string_literals;
                printer::fatal(
                    "unexpected error while parsing string literals: "s + e.what()
                );
                ::exit(EXIT_FAILURE);
            }
        } break;
        case U'\'': {
            try
            {
                ++i;
                if (data.at(i) == U'\\')
                    i += 2;
                else
                    ++i;
                if (data.at(i) != U'\'')
                {
                    printer::error(
                        "unterminated character literal"
                    );
                    errcount++;
                    goto end;
                }
                ++i;
            }
            catch (const std::out_of_range& e)
            {
                printer::error(
                    "unexpected end of file while parsing character literals"
                );
                errcount++;
                goto end;
            }
            catch (const std::exception& e)
            {
                using namespace std::string_literals;
                printer::fatal(
                    "unexpected error while parsing character literals: "s + e.what()
                );
                ::exit(EXIT_FAILURE);
            }
        } break;
        default:
            break;
        }
    }

end:
    if (errcount > 0)
    {
        auto&& mkdir_p = [](const stdfs::path& p)
        {
            if (!stdfs::exists(p))
                stdfs::create_directories(p);
        };
        mkdir_p("/tmp/supdef/stage2/");
        using namespace std::string_literals;
        printer::fatal(
            "found " + std::to_string(errcount) + " errors while parsing " + filename.get()->string()
        );
        // set timezome to user's timezone
        std::chrono::zoned_time zt = std::chrono::zoned_time(std::chrono::current_zone(), std::chrono::system_clock::now());
        // <dayname>-<day>-<month>-<year>-<hour>-<minute>-<second> (where dayname is the full name of the day of the week)
        std::string timestr = std::format(
            "{:%A-%d-%m-%Y-%H-%M-%S}",
            zt.get_local_time()
        );
        boost::filesystem::path unique = boost::filesystem::unique_path(
            "/tmp/supdef/stage2/"s + timestr + "-%%%%-%%%%-%%%%-%%%%.txt"
        );
        std::ofstream file(unique.c_str());
        file << format(data) << std::flush;
        printer::info(
            "stage2 output written to " + unique.string()
        );
        ::exit(EXIT_FAILURE);
    }
}

using supdef_map_type =
    std::unordered_multimap<
        std::u32string,
        ::supdef::parser::registered_supdef,
        ::supdef::detail::xxhash<std::u32string, 64>
    >;

namespace
{
    static ::supdef::parser::registered_supdef::options parse_supdef_options(const std::u32string& str)
    {
        // for now, no options are supported
        return ::supdef::parser::registered_supdef::options::none;
    }

    struct token_walker
    {
    private:
        using iterator = std::vector<::supdef::token>::const_iterator;
    public:
        token_walker(std::vector<::supdef::token>& tokens)
            : m_tokens(std::addressof(tokens))
            , m_index(0)
        {
        }
        const auto& next()
        {
            return m_tokens->at(m_index++);
        }
        const auto& peek()
        {
            return m_tokens->at(m_index);
        }
        const auto& peek_at(size_t index)
        {
            return m_tokens->at(index);
        }
        const auto& peek_at(iterator it)
        {
            return *it;
        }
        bool has_next() const
        {
            return m_index < m_tokens->size();
        }

        bool accept(::supdef::token_kind kind)
        {
            if (m_index >= m_tokens->size())
                return false;
            if (m_tokens->at(m_index).kind == kind)
            {
                ++m_index;
                return true;
            }
            return false;
        }

        bool accept_no_move(::supdef::token_kind kind)
        {
            if (m_index >= m_tokens->size())
                return false;
            return m_tokens->at(m_index).kind == kind;
        }

        bool expect(::supdef::token_kind kind)
        {
            if (m_index >= m_tokens->size())
                return false;
            if (m_tokens->at(m_index).kind == kind)
            {
                ++m_index;
                return true;
            }
            using namespace std::string_literals;
            // TODO: better error reporting, for example:
            //    - accept a message to print
            //    - accept a severity level
            //    - print default message below when no message is provided
            ::supdef::printer::error(
                "unexpected token: got "s
                + magic_enum::enum_name(m_tokens->at(m_index).kind).data()
                + " (" + format(m_tokens->at(m_index).data.value()) + ")"
                + " instead of " + magic_enum::enum_name(kind).data()
            );
            return false;
        }

        void remove(iterator it)
        {
            // if the iterator is before the current index, decrement the index
            if (it < m_tokens->begin() + m_index)
                --m_index;
            m_tokens->erase(it);
        }
        void remove(iterator first, iterator last)
        {
            assert(first <= last);
            // decrement the index by the number of elements removed before the current index
            iterator index_it = m_tokens->begin() + m_index;
            if (first < index_it)
                m_index -= std::min(
                    std::distance(first, index_it),
                    std::distance(first, last)
                );
            m_tokens->erase(first, last);
        }

        size_t skip_whitespaces(bool skip_newlines = false)
        {
            size_t count = 0;
            while (m_index < m_tokens->size())
            {
                if (m_tokens->at(m_index).kind == ::supdef::token_kind::horizontal_whitespace)
                {
                    ++m_index;
                    ++count;
                }
                else if (skip_newlines && m_tokens->at(m_index).kind == ::supdef::token_kind::newline)
                {
                    ++m_index;
                    ++count;
                }
                else
                    break;
            }
            return count;
        }

        size_t get_index() const
        {
            return m_index;
        }
        void set_index(size_t index)
        {
            m_index = index;
        }

        iterator get_iterator_at(size_t index = -1)
        {
            if (index == -1)
                index = m_index;
            return m_tokens->begin() + index;
        }

    private:
        std::vector<::supdef::token>* m_tokens;
        size_t m_index;
    };

    /*
     * TODO: instead of just `continue`ing everywhere, we should sometimes
     *       emit a warning or an error, depending on the situation.
     */

    // @ import "path"
    static std::vector<stdfs::path> find_imports(std::vector<::supdef::token>& tokens)
    {
        std::vector<stdfs::path> imports;
        token_walker walker(tokens);
        bool first_token = true;
        while (walker.has_next())
        {
            if (first_token)
            {
                first_token = false;
                goto no_check_needed;
            }
            if (!walker.accept(::supdef::token_kind::newline))
            {
                walker.next();
                continue;
            }
no_check_needed:
            walker.skip_whitespaces(true);
            if (!walker.accept(::supdef::token_kind::at))
                continue;
            auto at_index = walker.get_index() - 1;
            walker.skip_whitespaces();
            auto tok = walker.peek();
            if (!walker.accept(::supdef::token_kind::keyword) || tok.keyword != ::supdef::keyword_kind::import)
                continue;
            if (!walker.skip_whitespaces())
                supdef::printer::warning(
                    "expected at least one whitespace after import keyword"
                );
            auto potential_path = walker.peek();
            if (!walker.expect(::supdef::token_kind::string_literal))
                continue;
            auto import_end_index = walker.get_index();
            imports.emplace_back(
                format(potential_path.data.value())
            );
            walker.remove(
                walker.get_iterator_at(at_index),
                walker.get_iterator_at(import_end_index)
            );
        }
        return imports;
    }

    static inline std::optional<std::vector<::supdef::token>> parse_supdef_line(token_walker& walker)
    {
        std::vector<::supdef::token> line;
        if (!walker.accept(::supdef::token_kind::newline))
            return std::nullopt;
        walker.skip_whitespaces(true);
        auto tok = walker.next();
        if (tok.kind == ::supdef::token_kind::at)
        {
            if (walker.accept_no_move(::supdef::token_kind::keyword))
            {
                if (walker.peek().keyword == ::supdef::keyword_kind::end)
                    return std::nullopt;
            }
        }
        line.push_back(tok);
        //line.push_back(walker.peek());
        while (walker.has_next())
        {
            if (!walker.accept_no_move(::supdef::token_kind::newline))
                break;
            line.push_back(walker.next());
        }
        return line;
    }

    // @ supdef <options> begin <name>
    // <supdef-body>
    // @ end
    // <options> ::= ::supdef::token_kind::identifier*
    // <name> ::= ::supdef::token_kind::identifier
    // <supdef-body> ::= <supdef-line>*
    // <supdef-line> ::= <let-expresion> | <if-expression> | <for-expression> | <while-expression> | <expression>
    // <let-expression> ::= @let <identifier> = <expression>
    // <if-expression> ::= @if <expression> begin <expression> end

    static supdef_map_type find_supdefs(std::vector<::supdef::token>& tokens)
    {
        supdef_map_type supdefs;
        token_walker walker(tokens);
        bool first_token = true;
        while (walker.has_next())
        {
            ::supdef::parser::registered_supdef sd;
            std::u32string name;
            if (first_token)
            {
                first_token = false;
                goto no_check_needed;
            }
            if (!walker.accept(::supdef::token_kind::newline))
            {
                walker.next();
                continue;
            }
no_check_needed:
            walker.skip_whitespaces(true);
            if (!walker.accept(::supdef::token_kind::at))
                continue;
            auto at_index = walker.get_index() - 1;
            walker.skip_whitespaces();
            if (!walker.accept_no_move(::supdef::token_kind::keyword) || walker.next().keyword != ::supdef::keyword_kind::supdef)
                continue;
            walker.skip_whitespaces();
            if (!walker.accept_no_move(::supdef::token_kind::identifier) && !walker.accept_no_move(::supdef::token_kind::keyword))
                continue;
            auto begin_or_opts = walker.peek();
            if (begin_or_opts.keyword.value_or(::supdef::keyword_kind::unknown) == ::supdef::keyword_kind::begin)
                sd.opts = ::supdef::parser::registered_supdef::options::none;
            else
            {
                sd.opts = parse_supdef_options(format(begin_or_opts.data.value()));
                walker.skip_whitespaces();
                if (!walker.accept_no_move(::supdef::token_kind::keyword) || walker.next().keyword != ::supdef::keyword_kind::begin)
                    continue;
            }
            walker.skip_whitespaces();
            if (!walker.accept_no_move(::supdef::token_kind::identifier))
                continue;
            name = walker.next().data.value();
            walker.skip_whitespaces();
            while (true)
            {
                auto line = parse_supdef_line(walker);
                if (!line.has_value())
                    break;
                sd.lines.push_back(line);
            }
            walker.skip_whitespaces(true);
            if (!walker.accept(::supdef::token_kind::at))
                continue;
            walker.skip_whitespaces();
            if (!walker.accept_no_move(::supdef::token_kind::keyword) || walker.next().keyword != ::supdef::keyword_kind::end)
                continue;
            walker.remove(
                walker.get_iterator_at(at_index),
                walker.get_iterator_at(walker.get_index())
            );
            supdefs.emplace(name, std::move(sd));
        }
        return supdefs;
    }
}

void supdef::parser::do_stage3()
{
    tokenizer tkn(m_file.data());
    for (auto&& tok : tkn.tokenize())
        m_tokens.push_back(tok);

    std::vector imports = find_imports(m_tokens);
    auto already_processed = [this](const stdfs::path& p)
    {
        auto& files = ::supdef::globals::get_already_processed_files();
        return std::find_if(
            files.begin(), files.end(),
            [&p](const auto& x) { return *x == p; }
        ) != files.end();
    };
    auto canon = [](const stdfs::path& p)
    { return stdfs::canonical(p); };
    for (auto&& import : imports)
    {
        auto canon_import = canon(import);
        std::cout << "importing: " << canon_import.string() << '\n';
        if (already_processed(canon_import))
            continue;
        parser p(canon_import);
        p.do_stage1();
        p.do_stage2();
        p.do_stage3();
        m_imported_parsers.insert(p);
    }

    // TODO: find and parse supdef's
    std::unordered_multimap supdefs = find_supdefs(m_tokens);
    this->m_supdefs.merge(std::move(supdefs));
    // TODO: find and parse runnable's
    // TODO: find and parse embed's
    // TODO: find and parse dump's
}

void supdef::parser::output_to(std::ostream& os, output_kind kind)
{
    if (kind & output_kind::text)
        // TODO: output different things depending on current stage
        os << format(m_file.data());
    if (kind & output_kind::tokens)
    {
        auto enum_name_as_string = [](token& e) static constexpr noexcept -> std::string_view
        {
            using namespace std::string_view_literals;
            std::string_view ret = e.keyword.has_value() ?
                magic_enum::enum_name(e.keyword.value()) :
                "none"sv;
            return ret;
        };
        auto fmt_data = [](std::optional<std::u32string>& data) static constexpr noexcept -> std::string
        {
            using namespace std::string_literals;
            return data.has_value()  ?
                format(data.value()) :
                "none"s;
        };
        auto fmt_whitespace = [](token& e) static constexpr noexcept -> std::string
        {
            using namespace std::string_literals;
            assert(e.data.has_value() && e.data.value().size() == 1);
            auto echar = static_cast<uint32_t>(e.data.value().at(0));
            return e.kind == token_kind::newline                ?
                "<newline>: U+"s + std::format("{:04X}", echar) :
                "<horizontal whitespace>: U+"s + std::format("{:04X}", echar);
        };
        auto fmt = [fmt_data, fmt_whitespace](token& e) constexpr noexcept -> std::string
        {
            if (e.kind == token_kind::newline || e.kind == token_kind::horizontal_whitespace)
                return fmt_whitespace(e);
            else
                return fmt_data(e.data);
        };
        size_t i = 0;
        for (auto&& tok : m_tokens)
            os << "<token n°" << i++ << ">\n"
               << "\tkind:\n\t\t" << magic_enum::enum_name(tok.kind) << '\n'
               << "\tdata:\n\t\t" << fmt(tok) << '\n'
               << "\tkeyword:\n\t\t" << enum_name_as_string(tok) << '\n';
    }
    if (kind & output_kind::ast)
    {
        // TODO
    }
    os << std::flush;
}

void supdef::parser::output_to(const stdfs::path& filename, output_kind kind)
{
    std::ofstream file(filename);
    output_to(file, kind);
}

#endif