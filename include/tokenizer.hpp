#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

#include <types.hpp>
#include <unicode.hpp>

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
        boolean_literal,        // true or false

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
        semicolon,             // ;
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
        hash,                  // #

        backslash,             // <U+005C>
        inline_comment,        // // ...
        multiline_comment,     // /* ... */

        other,      // any other token
        line_start, // start of line
        eof         // end of file
    };

    enum class keyword_kind
    {
        __unused = -1,

        import, embed, dump,

        supdef, runnable,
        begin, end,

        set, unset, pragma,
        global, defer,
        
        if_, elseif, else_, endif,
        for_, endfor,
        foreach, foreachi, endforeach,
        
        join, split,
        str, unstr,
        len, math,
        raw,

        unknown
    };

    struct token_loc
    {
        shared_ptr<const stdfs::path> filename;
        size_t line;
        size_t column;

        std::make_unsigned_t<off_t> infile_offset;
        size_t toksize;

        std::string to_string() const
        {
            return filename->string()   + ":" +
                   std::to_string(line) + ":" +
                   std::to_string(column);
        }
    };

    struct token
    {
        token_loc loc;
        std::optional<std::u32string> data;
        std::optional<keyword_kind> keyword;
        token_kind kind;
    };

    class tokenizer
    {
    public:
        tokenizer(const std::u32string &data, shared_ptr<const stdfs::path> filename = nullptr);
        ~tokenizer();

        std::generator<token> tokenize(shared_ptr<const stdfs::path> filename = nullptr);

    private:
        std::u32string m_data;
        shared_ptr<const stdfs::path> m_filename;
    };

    std::string description_string(token_kind kind);
    std::string description_string(keyword_kind kind);
}

#endif
