#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

#include <types.hpp>
#include <unicode.hpp>

#include <filesystem>
#include <vector>
#include <utility>
#include <generator>
#include <string>

namespace supdef
{
    enum class token_kind
    {
        identifier,             // [\p{L}_][\p{L}\p{N}_]*
        keyword,                // [supdef keyword]
        string_literal,         // "..."
        char_literal,           // '...'
        integer_literal,        // [0-9]+
        floating_literal,       // [0-9]+\.[0-9]+([eE][+-]?[0-9]+)?
        hex_integer_literal,    // 0x[0-9a-fA-F]+
        binary_integer_literal, // 0b[01]+
        octal_integer_literal,  // 0[0-7]+

        newline,               // \n or equivalent
        horizontal_whitespace, // \h
        lparen,                // (
        rparen,                // )
        lbrace,                // {
        rbrace,                // }
        lbracket,              // [
        rbracket,              // ]
        langle,                // <
        rangle,                // >
        comma,                 // ,
        colon,                 // :
        equals,                // =
        plus,                  // +
        minus,                 // -
        asterisk,              // *
        slash,                 // /
        percent,               // %
        ampersand,             // &
        pipe,                  // |
        caret,                 // ^
        exclamation,           // !
        question,              // ?
        dollar,                // $
        at,                    // @
        tilde,                 // ~
        period,                // .

        other, // any other token
        eof    // end of file
    };

    enum class keyword_kind
    {
        __unused = -1,

        supdef, import, runnable,
        embed, dump, set, if_,
        elseif, else_, endif,
        for_, end, join, split,
        str, unstr, len, math,
        begin, unset, foreach,
        foreachi, raw,

        unknown
    };

    struct token
    {
        std::optional<std::u32string> data;
        std::optional<keyword_kind> keyword;
        token_kind kind;
    };

    class tokenizer
    {
    public:
        tokenizer(const std::u32string &data);
        ~tokenizer();

        std::generator<token> tokenize();

    private:
        std::u32string m_data;
    };
}

#endif
