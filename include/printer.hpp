#ifndef PRINTER_HPP
#define PRINTER_HPP

#include <types.hpp>
#include <tokenizer.hpp>
#include <version.hpp>

#include <bits/stdc++.h>

#include <simdutf.h>

namespace supdef
{
    namespace printer
    {
        static std::u32string unformat(const std::string& str)
        {
            if (str.empty())
                return U"";

            size_t required_length = simdutf::utf32_length_from_utf8(str.data(), str.size());
            std::unique_ptr<char32_t[]> buf(new char32_t[required_length]);
            size_t result = simdutf::convert_valid_utf8_to_utf32(str.data(), str.size(), buf.get());
            if (result == 0)
                throw std::runtime_error("failed to convert utf8 to utf32");

            return std::u32string(buf.get(), result);
        }

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

        static std::string format(const std::u16string& str, size_t start = 0, size_t end = std::u16string::npos)
        {
            const auto& dataptr = str.data();
            const auto& datalen = str.size();
            const auto& datastart = dataptr + start;

            if (end == std::u16string::npos)
                end = datalen;
            
            if (str.empty())
                return "";

            size_t required_length = simdutf::utf8_length_from_utf16(datastart, end - start);
            std::unique_ptr<char[]> buf(new char[required_length]);
            size_t result = simdutf::convert_valid_utf16_to_utf8(datastart, end - start, buf.get());
            if (result == 0)
                throw std::runtime_error("failed to convert utf16 to utf8");

            return std::string(buf.get(), result);
        }

#define __BOLDRED  "\033[1;31m"
#define __RED   "\033[31m"
#define __YELLOW "\033[33m"
#define __BLUE  "\033[34m"
#define __RESET "\033[0m"

#define __INFO  __BLUE
#define __WARN  __YELLOW
#define __ERR   __RED
#define __FATAL __BOLDRED

        static inline void info(const std::string& msg)
        {
            using namespace std::string_literals;
            std::clog << __INFO << "info:"s + __RESET + " " << msg << '\n';
        }

        static inline void warning(const std::string& msg)
        {
            using namespace std::string_literals;
            std::clog << __WARN << "warning:"s + __RESET + " " << msg << '\n';
        }

        static inline void error(const std::string& msg)
        {
            using namespace std::string_literals;
            std::cerr << __ERR << "error:"s + __RESET + " " << msg << '\n';
        }

        static inline void fatal(const std::string& msg)
        {
            using namespace std::string_literals;
            std::cerr << __FATAL << "fatal error:"s + __RESET + " " << msg << '\n';
        }

        using format_fn = std::string(*)(const std::u32string&, size_t, size_t);
        static inline void info(
            const std::string& msg, const token& tok, const std::u32string& orig,
            format_fn fmt = &format
        ) {
            auto linestr = std::to_string(tok.loc.line);
            auto colstr  = std::to_string(tok.loc.column);
            size_t linestart = orig.rfind(U'\n', tok.loc.infile_offset);
            size_t lineend = orig.find(U'\n', tok.loc.infile_offset);
            linestart = linestart != std::u32string::npos ?
                        linestart                         :
                        0;
            lineend   = lineend   != std::u32string::npos ?
                        lineend                           :
                        orig.size();
            using namespace std::string_literals;
            std::clog << __INFO << "info:"s + __RESET + " at " <<
                tok.loc.filename->string() << ':' << linestr << ':' << colstr << '\n' <<
                msg << '\n' <<
                "  " << linestr << " | " << fmt(orig, linestart, lineend) << '\n' <<
                "  " << std::string(linestr.size(), ' ') << " | " << std::string(tok.loc.column - 1, ' ') << __INFO << "^"s + __RESET << '\n';
        }

        static inline void warning(
            const std::string& msg, const token& tok, const std::u32string& orig,
            format_fn fmt = &format
        ) {
            auto linestr = std::to_string(tok.loc.line);
            auto colstr  = std::to_string(tok.loc.column);
            size_t linestart = orig.rfind(U'\n', tok.loc.infile_offset) + 1;
            size_t lineend = orig.find(U'\n', tok.loc.infile_offset);
            linestart = linestart != std::u32string::npos ?
                        linestart                         :
                        0;
            lineend   = lineend   != std::u32string::npos ?
                        lineend                           :
                        orig.size();
            using namespace std::string_literals;
            std::clog << __WARN << "warning:"s + __RESET + " at " <<
                tok.loc.filename->string() << ':' << linestr << ':' << colstr << '\n' <<
                msg << '\n' <<
                "  " << linestr << " | " << fmt(orig, linestart, lineend) << '\n' <<
                "  " << std::string(linestr.size(), ' ') << " | " << std::string(tok.loc.column - 1, ' ') << __WARN << "^"s + __RESET << '\n';
        }

        static inline void error(
            const std::string& msg, const token& tok, const std::u32string& orig,
            format_fn fmt = &format
        ) {
            auto linestr = std::to_string(tok.loc.line);
            auto colstr  = std::to_string(tok.loc.column);
            size_t linestart = orig.rfind(U'\n', tok.loc.infile_offset) + 1;
            size_t lineend = orig.find(U'\n', tok.loc.infile_offset);
            linestart = linestart != std::u32string::npos ?
                        linestart                         :
                        0;
            lineend   = lineend   != std::u32string::npos ?
                        lineend                           :
                        orig.size();
            using namespace std::string_literals;
            std::cerr << __ERR << "error:"s + __RESET + " at " <<
                tok.loc.filename->string() << ':' << linestr << ':' << colstr << '\n' <<
                msg << '\n' <<
                "  " << linestr << " | " << fmt(orig, linestart, lineend) << '\n' <<
                "  " << std::string(linestr.size(), ' ') << " | " << std::string(tok.loc.column - 1, ' ') << __ERR << "^"s + __RESET << '\n';
        }

        static inline void fatal(
            const std::string& msg, const token& tok, const std::u32string& orig,
            format_fn fmt = &format
        ) {
            auto linestr = std::to_string(tok.loc.line);
            auto colstr  = std::to_string(tok.loc.column);
            size_t linestart = orig.rfind(U'\n', tok.loc.infile_offset) + 1;
            size_t lineend = orig.find(U'\n', tok.loc.infile_offset);
            linestart = linestart != std::u32string::npos ?
                        linestart                         :
                        0;
            lineend   = lineend   != std::u32string::npos ?
                        lineend                           :
                        orig.size();
            using namespace std::string_literals;
            std::cerr << __FATAL << "fatal error:"s + __RESET + " at " <<
                tok.loc.filename->string() << ':' << linestr << ':' << colstr << '\n' <<
                msg << '\n' <<
                "  " << linestr << " | " << fmt(orig, linestart, lineend) << '\n' <<
                "  " << std::string(linestr.size(), ' ') << " | " << std::string(tok.loc.column - 1, ' ') << __FATAL << "^"s + __RESET << '\n';
        }

#undef  __BOLDRED
#undef  __RED
#undef  __YELLOW
#undef  __BLUE
#undef  __RESET

#undef  __INFO
#undef  __WARN
#undef  __ERR
#undef  __FATAL
    }
}

#endif
