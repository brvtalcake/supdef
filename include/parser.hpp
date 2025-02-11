#ifndef PARSER_HPP
#define PARSER_HPP

#include <types.hpp>
#include <stack.hpp>
#include <interpreter.hpp>
#include <file.hpp>
#include <unicode.hpp>
#include <tokenizer.hpp>
#include <detail/xxhash.hpp>

#include <filesystem>
#include <vector>
#include <list>
#include <set>
#include <utility>
#include <generator>
#include <string>
#include <string_view>
#include <optional>
#include <memory>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <functional>
#include <stdexcept>
#include <map>
#include <variant>
#include <regex>

#include <unicode/ustream.h>
#include <unicode/unistr.h>

#include <boost/logic/tribool.hpp>

namespace supdef
{
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
        static inline void info(const std::string& msg, const token& tok, const std::u32string& orig, format_fn format)
        {
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
                "  " << linestr << " | " << format(orig, linestart, lineend) << '\n' <<
                "  " << std::string(linestr.size(), ' ') << " | " << std::string(tok.loc.column - 1, ' ') << __INFO << "^"s + __RESET << '\n';
        }

        static inline void warning(const std::string& msg, const token& tok, const std::u32string& orig, format_fn format)
        {
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
                "  " << linestr << " | " << format(orig, linestart, lineend) << '\n' <<
                "  " << std::string(linestr.size(), ' ') << " | " << std::string(tok.loc.column - 1, ' ') << __WARN << "^"s + __RESET << '\n';
        }

        static inline void error(const std::string& msg, const token& tok, const std::u32string& orig, format_fn format)
        {
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
                "  " << linestr << " | " << format(orig, linestart, lineend) << '\n' <<
                "  " << std::string(linestr.size(), ' ') << " | " << std::string(tok.loc.column - 1, ' ') << __ERR << "^"s + __RESET << '\n';
        }

        static inline void fatal(const std::string& msg, const token& tok, const std::u32string& orig, format_fn format)
        {
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
                "  " << linestr << " | " << format(orig, linestart, lineend) << '\n' <<
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

    struct registered_base
    {
        static constexpr boost::logic::tribool parse_bool_opt(
            std::u32string_view sv, std::u32string_view opt
        );
    };

    class parser
    {
        struct parser_compare
        {
            using ret_type = decltype(std::declval<stdfs::path>() <=> std::declval<stdfs::path>());

            enum class op
            {
                less,
                greater,
                equal
            };
        protected:
            static bool do_less(const stdfs::path& lhs, const stdfs::path& rhs)
            {
                return lhs < rhs;
            }

        public:
            static bool operator()(const parser& lhs, const parser& rhs, op op = op::less)
            {
                if (op == op::less)
                    return do_less(*lhs.m_file.filename(), *rhs.m_file.filename());
                std::unreachable(); // not implemented
            }
        };

        friend parser_compare;

        template <typename ParsedT>
        using parser_return = std::tuple<
            std::list<token>::const_iterator,
            std::list<token>::const_iterator,
            std::optional<ParsedT>
        >;

        [[__nodiscard__]]
        bool add_child_parser(const stdfs::path& filename, token_kind pathtype) noexcept;

    public:
        struct registered_supdef
            : private registered_base
        {
            PACKED_STRUCT(options)
            {
                unsigned eat_newlines : 1;
            };

            static constexpr options parse_options(const std::u32string& str);

            options opts;
            std::u32string name;
            std::vector<std::vector<::supdef::token>> lines;
        };

        struct registered_runnable
            : private registered_base
        {
            TODO(make it possible to use e.g. "@pragma compiler_path C <path>");
            struct lang
            {
                struct cmdline_source_placeholder { };
                struct cmdline_executable_placeholder { };
                struct cmdline_compiler_placeholder { };
                struct cmdline_interpreter_placeholder { };
                struct cmdline_version_placeholder { };
                struct cmdline_joinargs_placeholder { };
            
                using cmdline_args_type = std::variant<
                    cmdline_source_placeholder,
                    cmdline_executable_placeholder,
                    cmdline_compiler_placeholder,
                    cmdline_interpreter_placeholder,
                    cmdline_version_placeholder,
                    cmdline_joinargs_placeholder,
                    std::u32string
                >;

                struct execinfo
                {
                    // the standard / version used / needed
                    std::u32string version;

                    // the compiler and interpreter used, if needed
                    std::optional<stdfs::path> compiler, interpreter;

                    // the options if needed
                    std::vector<cmdline_args_type> compiler_cmdline, execution_cmdline;
                } exinfo;

                // the language used
                enum identifier
                {
                    c, cpp, rust, d, zig,
                    csharp, fsharp,
                    java,
                    ocaml, racket, haskell,
                    python, shell, perl, ruby
                } ident;
            };

            PACKED_STRUCT(options)
            {
                enum class mode : unsigned
                {
                    compileok  = 0b00,
                    trycompile = compileok,

                    runok      = 0b01,
                    tryrun     = runok,

                    getoutput  = 0b10,

                    retval     = 0b11
                };
                unsigned eat_newlines : 1;
                unsigned mode : 2;
            };

            static constexpr options parse_options(const std::u32string& str);
            static constexpr lang parse_lang(const std::u32string& str);

            lang langinfo;
            options opts;
            std::u32string name;
            std::vector<std::vector<::supdef::token>> lines;
        };

        struct subsitution_context
        {
            umap<std::u32string, std::list<token>> variables;
            /* umultimap<registered_runnable::lang::identifier, std::u32string> lang_aliases; */
            umap<registered_runnable::lang::identifier, registered_runnable::lang::execinfo> default_langinfo;
            std::optional<registered_supdef::options> default_sdopts;
            std::optional<registered_runnable::options> default_runopts;
            umap<std::u32string, registered_supdef>* supdefs;
            umap<std::u32string, registered_runnable>* runnables;
            bool toplevel;
        };
    
    protected:
        static std::optional<std::pair<registered_supdef::options, std::u32string>>
        parse_supdef_start(
            std::list<token>::const_iterator line_start,
            std::list<token>::const_iterator line_end,
            const std::u32string& origdata
        );

        static std::optional<
            std::tuple<registered_runnable::lang, registered_runnable::options, std::u32string>
        >
        parse_runnable_start(
            std::list<token>::const_iterator line_start,
            std::list<token>::const_iterator line_end,
            const std::u32string &origdata
        );

        static bool
        parse_supdef_runnable_end(
            std::list<token>::const_iterator line_start,
            std::list<token>::const_iterator line_end,
            const std::u32string& origdata
        );

        std::optional<registered_supdef> get_supdef(const std::u32string& name, bool recurse = true) const noexcept;

    public:

        parser(const stdfs::path& filename);
        parser(stdfs::path&& filename);
        ~parser();

        // tokenize the file
        void do_stage1();
        // splice lines
        void do_stage2();
        // remove comments
        void do_stage3();
        // process imports
        void do_stage4();
        // retrieve supdefs and runnables
        void do_stage5();
        // substitute supdefs, runnables or builtin functions (e.g. @math, @len, ...) calls
        // substitute @set variables
        // parse @pragma's
        void do_stage6();
        // process @embed and @dump calls
        void do_stage7();

        enum output_kind : uint_fast8_t
        {
            text      = 1U << 0,
            tokens    = 1U << 1,
            ast       = 1U << 2,
            imports   = 1U << 3,
            supdefs   = 1U << 4,
            recursive = 1U << 6,
            original  = 1U << 7,

            recursive_supdefs = supdefs
                              | recursive,
            all               = text
                              | tokens
                              | ast
                              | imports
                              | supdefs
                              | recursive
                              | original
        };

        void output_to(std::ostream& os, output_kind kind = text);
        void output_to(const std::filesystem::path& filename, output_kind kind = text);

        parser_compare::ret_type operator<=>(const parser& rhs) const
        {
            auto thispath = *m_file.filename();
            auto rhspath  = *rhs.m_file.filename();
            assert(thispath.is_absolute());
            assert(rhspath.is_absolute());
            return thispath <=> rhspath;
        }
    private:
        using supdef_map_type   = umap<std::u32string, registered_supdef>;
        using runnable_map_type = umap<std::u32string, registered_runnable>;
        stack<subsitution_context> m_ctx;
        source_file m_file;
        std::list<token> m_tokens;
        std::set<parser> m_imported_parsers;
        supdef_map_type m_supdefs;
        runnable_map_type m_runnables;
    };
}

#include <impl/parser.tpp>

#endif