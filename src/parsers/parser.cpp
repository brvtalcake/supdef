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

supdef::parser::parser(const stdfs::path& filename)
    : m_file(filename)
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

namespace
{
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
            // TODO: better error reporting
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

    // @ import "path"
    static std::vector<stdfs::path> find_imports(std::vector<::supdef::token>& tokens)
    {
        std::vector<stdfs::path> imports;
        token_walker walker(tokens);
        while (walker.has_next())
        {
            if (!walker.accept(::supdef::token_kind::at))
                continue;
            auto at_index = walker.get_index() - 1;
            walker.skip_whitespaces();
            if (!walker.accept(::supdef::token_kind::keyword))
                continue;
            if (walker.peek().keyword.value() != ::supdef::keyword_kind::import)
                continue;
            if (!walker.skip_whitespaces(false))
                supdef::printer::warning(
                    "expected at least one whitespace after import keyword"
                );
            if (!walker.expect(::supdef::token_kind::string_literal))
                continue;
            auto import_end_index = walker.get_index();
            imports.emplace_back(
                format(walker.peek().data.value())
            );
            walker.remove(
                walker.get_iterator_at(at_index),
                walker.get_iterator_at(import_end_index)
            );
        }
        return imports;
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
        if (already_processed(canon_import))
            continue;
        parser p(canon_import);
        p.do_stage1();
        p.do_stage2();
        p.do_stage3();
        m_imported_parsers.insert(p);
    }

    // TODO: find and parse supdef's
    // TODO: find and parse runnable's
    // TODO: find and parse embed's
    // TODO: find and parse dump's
}

void supdef::parser::output_to(std::ostream& os, output_kind kind)
{
    if (kind & output_kind::text)
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