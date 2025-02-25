#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP

#include <types.hpp>
#include <unicode.hpp>
#include <file.hpp>
#include <tokenizer.hpp>

#include <variant>
#include <string>
#include <optional>
#include <vector>
#include <map>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <algorithm>
#include <iterator>
#include <memory>
#include <utility>

#include <boost/multiprecision/gmp.hpp>
#include <boost/multiprecision/mpfr.hpp>

namespace supdef
{
#if 0
    struct registered_base
    {
        static constexpr boost::logic::tribool parse_bool_val(
            std::u32string_view sv
        );
        static constexpr boost::logic::tribool parse_bool_opt(
            std::u32string_view sv, std::u32string_view opt
        );
    };

    struct registered_supdef
        : private registered_base
    {
        PACKED_STRUCT(options)
        {
            unsigned eat_newlines : 1;
        };

        static constexpr options none_options = options{.eat_newlines = 0};

        static constexpr options parse_options(const std::u32string &str);

        options opts;
        std::u32string name;
        std::vector<std::vector<::supdef::token>> lines;
    };

    struct registered_runnable
        : private registered_base
    {
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
                std::u32string>;

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
            enum class identifier
            {
                c = 1, cpp, rust, d, zig,
                csharp, fsharp,
                java,
                ocaml, racket, haskell,
                python, shell, perl, ruby
            } ident;
            static constexpr size_t _langidentcount = static_cast<size_t>(identifier::ruby);
        };

        PACKED_STRUCT(options)
        {
            enum class mode : unsigned
            {
                compileok = 0b00,
                trycompile = compileok,

                runok = 0b01,
                tryrun = runok,

                getoutput = 0b10,

                retval = 0b11
            };
            unsigned eat_newlines : 1;
            unsigned mode : 2;
        };

        static constexpr std::optional<lang::identifier> is_lang_identifier(std::u32string_view sv);
        static constexpr options parse_options(const std::u32string &str);
        static constexpr lang parse_lang(const std::u32string &str);

        lang langinfo;
        options opts;
        std::u32string name;
        std::vector<std::vector<::supdef::token>> lines;
    };

    struct substitution_context
    {
        umap<std::u32string, std::list<token>> variables;
        /* umultimap<registered_runnable::lang::identifier, std::u32string> lang_aliases; */
        umap<registered_runnable::lang::identifier, registered_runnable::lang::execinfo> default_langinfo;
        std::optional<registered_supdef::options> default_sdopts;
        std::optional<registered_runnable::options> default_runopts;
        umap<std::u32string, registered_supdef> *supdefs;
        umap<std::u32string, registered_runnable> *runnables;
        std::vector<std::list<token>> arguments;
        bool toplevel, in_supdef, in_runnable;
    };
#else
    struct substitution_context
    {
        
    };
#endif
    namespace interpreter
    {
        /* using sdinteger = bigint;
        using sdfloat   = bigfloat;
        using sdstring  = std::u32string;
        using sdtokens  = std::list<token>;

        using sdatom = std::variant<
            sdinteger,  // integer
            sdfloat,    // float
            sdstring,   // string
            sdtokens    // tokens
        >; */

        struct supdef_params
        {
            bool eat_newlines;
        };

        struct runnable_params
        {
            enum class mode : unsigned
            {
                compileok = 0b00,
                trycompile = compileok,

                runok = 0b01,
                tryrun = runok,

                getoutput = 0b10,

                retval = 0b11
            } mode;
            bool eat_newlines;
        };

        struct runnable_lang_params
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
                std::u32string>;

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
            enum class identifier
            {
                c = 1, cpp, rust, d, zig,
                csharp, fsharp,
                java,
                ocaml, racket, haskell,
                python, shell, perl, ruby
            } ident;
            static constexpr size_t _langidentcount = static_cast<size_t>(identifier::ruby);
        };
    }
}

#endif
