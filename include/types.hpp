#ifndef TYPES_HPP
#define TYPES_HPP

#include <concepts>
#include <type_traits>
#include <source_location>
#include <filesystem>
#include <cstddef>
#include <cstdint>

#include <experimental/scope>

#include <unicode.hpp>

namespace stdfs = ::std::filesystem;
namespace stdx = ::std::experimental;
namespace stdext = ::__gnu_cxx;

namespace supdef
{
    struct location
    {
        std::shared_ptr<const stdfs::path> filename;
        size_t line;
        size_t column;
        size_t off;
        size_t len;
    };

    enum token_type
    {
        TOK_LPAREN, TOK_RPAREN,               // ( )
        TOK_LBRACE, TOK_RBRACE,               // { }
        TOK_LBRACKET, TOK_RBRACKET,           // [ ]
        //TOK_CCOMMENT_START, TOK_CCOMMENT_END, // /* */
        //TOK_CPPCOMMENT,                       // //
        TOK_CHARLIT, TOK_STRINGLIT,           // ' ' " "
        TOK_DOLLAR,                           // $
        TOK_SUPDEF,                           // @\s*supdef\s+<options>\s+begin\s+<name>
        TOK_IDENTIFIER,                       // [a-zA-Z_][a-zA-Z0-9_]*
        TOK_OTHER,                            // Any other token
        TOK_EOF                               // End of file
    };

    struct token
    {
        token_type type;
        std::reference_wrapper<const icu::UnicodeString> origtxt;
        location loc;
    };
}

#endif
