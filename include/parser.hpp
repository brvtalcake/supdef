#ifndef PARSER_HPP
#define PARSER_HPP

#include <types.hpp>
#include <directives.hpp>
#include <file.hpp>

#include <filesystem>
#include <vector>
#include <utility>
#include <generator>

#include <unicode/ustream.h>
#include <unicode/unistr.h>

#if CUSTOM_FORMATTER
template <>
struct std::formatter<supdef::location>
{
public:
    
    // format
    template<class FmtContext>
    FmtContext::iterator format(const supdef::location& loc, FmtContext& ctx) const
    {
        return std::format_to(
            ctx.out(),
            "{}:{}:{}",
            loc.filename->string(),
            loc.line,
            loc.column
        );
    }

    // parse
    template<class ParseContext>
    constexpr ParseContext::iterator parse(ParseContext& ctx) const
    {
        auto it = ctx.begin();
        if (it == ctx.end())
            return it;
 
        return it; // TODO
    }
};
#endif

namespace supdef
{
#if 0
    template <typename T, typename R, typename... Args>
    concept has_format = requires(T t, Args... args)
    {
        { t.format(args...) } -> std::convertible_to<R>;
    };

    template <typename CRTP>
        /* requires has_format<CRTP, std::string, const std::string&> */
    class parse_error
        : public std::exception
    {
    public:
        enum class severity
        {
            warning,
            error,
            fatal
        };

        parse_error(severity type, const std::string& msg)
            : m_msg(msg), m_type(type)
        {
        }

        parse_error(severity type, std::string&& msg)
            : m_msg(std::move(msg)), m_type(type)
        {
        }

        virtual ~parse_error()
        {
            if (m_buffer)
                delete[] m_buffer;
        }

        virtual const char* what() const noexcept override
        {
            auto str = dynamic_cast<const CRTP*>(this)->format(m_msg);
            const char* cstr = str.c_str();
            if (m_buffer)
                delete[] m_buffer;
            m_buffer = new char[strlen(cstr) + 1];
            strcpy(m_buffer, cstr);
            return m_buffer;
        }

        virtual const char* what() noexcept
        {
            auto str = dynamic_cast<CRTP*>(this)->format(m_msg);
            const char* cstr = str.c_str();
            if (m_buffer)
                delete[] m_buffer;
            m_buffer = new char[strlen(cstr) + 1];
            strcpy(m_buffer, cstr);
            return m_buffer;
        }

    private:
        std::string m_msg;
        mutable char* m_buffer = nullptr;
    protected:
        severity m_type;
    };

    class stage1_error
        : public parse_error<stage1_error>
    {
    public:
        stage1_error(severity type, const std::string& msg, const icu::UnicodeString& origtxt, location loc)
            : parse_error(type, msg), m_origtxt(std::ref(origtxt)), m_loc(loc)
        {
        }

        stage1_error(severity type, std::string&& msg, const icu::UnicodeString& origtxt, location loc)
            : parse_error(type, std::move(msg)), m_origtxt(std::ref(origtxt)), m_loc(loc)
        {
        }

        std::string format(const std::string& str) const
        {
            std::string buffer;
            m_origtxt.get().tempSubString(m_loc.off, m_loc.len).toUTF8String(buffer);
            return std::format(
#if CUSTOM_FORMATTER
                "stage 1: {} at {}: {}\n"
#else
                "stage 1: {} at {}:{}:{}: {}\n"
#endif
                "  {}\n",
                m_type == severity::warning ?
                    "warning" :
                    m_type == severity::error ?
                        "error" :
                        "fatal error",
#if CUSTOM_FORMATTER
                m_loc,
#else
                m_loc.filename->string(),
                m_loc.line,
                m_loc.column,
#endif
                str,
                buffer
            );
        }

        severity get_severity() const
        {
            return m_type;
        }

    private:
        std::reference_wrapper<const icu::UnicodeString> m_origtxt;
        location m_loc;
    };

    class stage2_error
        : public parse_error<stage2_error>
    {
    public:
        stage2_error(severity type, const std::string& msg, const icu::UnicodeString& origtxt, location loc)
            : parse_error(type, msg), m_origtxt(std::ref(origtxt)), m_loc(loc)
        {
        }

        stage2_error(severity type, std::string&& msg, const icu::UnicodeString& origtxt, location loc)
            : parse_error(type, std::move(msg)), m_origtxt(std::ref(origtxt)), m_loc(loc)
        {
        }

        std::string format(const std::string& str) const
        {
            std::string buffer;
            m_origtxt.get().tempSubString(m_loc.off, m_loc.len).toUTF8String(buffer);
            return std::format(
#if CUSTOM_FORMATTER
                "stage 2: {} at {}: {}\n"
#else
                "stage 2: {} at {}:{}:{}: {}\n"
#endif
                "  {}\n",
                m_type == severity::warning ?
                    "warning" :
                    m_type == severity::error ?
                        "error" :
                        "fatal error",
#if CUSTOM_FORMATTER
                m_loc,
#else
                m_loc.filename->string(),
                m_loc.line,
                m_loc.column,
#endif
                str,
                buffer
            );
        }

        severity get_severity() const
        {
            return m_type;
        }

    private:
        std::reference_wrapper<const icu::UnicodeString> m_origtxt;
        location m_loc;
    };
#else
    namespace printer
    {
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
            std::cerr << __INFO << "info:"s + __RESET + " " << msg << '\n';
        }

        static inline void warning(const std::string& msg)
        {
            using namespace std::string_literals;
            std::cerr << __WARN << "warning:"s + __RESET + " " << msg << '\n';
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
#endif

    class parser
    {
    public:
        parser(const stdfs::path& filename);
        ~parser();

#if 0
        std::generator<stage1_error> do_stage1();
        std::generator<stage2_error> do_stage2();
#else
        void do_stage1();
        void do_stage2();
#endif

        void output_to(std::ostream& os);
        void output_to(const std::filesystem::path& filename);

    private:
        source_file m_file;
    };
}

#endif
