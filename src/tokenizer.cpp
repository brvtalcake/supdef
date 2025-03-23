/* class tokenizer
{
public:
    tokenizer(const std::u32string& data);
    ~tokenizer();

    std::generator<token> tokenize();

private:
    std::u32string m_data;
}; */

#include <printer.hpp>
#include <tokenizer.hpp>
#include <unicode.hpp>
#include <detail/ckd_arith.hpp>
#include <impl/parsing-utils.tpp>

#include <simdutf.h>

#include <magic_enum.hpp>

#include <boost/preprocessor.hpp>

#undef  __CKD_DISTANCE
#define __CKD_DISTANCE(memb, ...)   \
    ::supdef::checked_cast<         \
        decltype(                   \
            std::declval<           \
                ::supdef::token_loc \
            >().memb                \
        )                           \
    >(std::distance(__VA_ARGS__))

namespace
{
    using ::supdef::printer::format;

    static inline std::string format(const std::u32string::const_iterator& start, const std::u32string::const_iterator& end)
    {
        return format(std::u32string(start, end));
    }

    static inline std::string format(const std::u32string::iterator& start, const std::u32string::iterator& end)
    {
        return format(std::u32string(start, end));
    }
}

supdef::tokenizer::tokenizer(const std::u32string& data, ::supdef::shared_ptr<const stdfs::path> filename)
    : m_data(data), m_filename(filename)
{
}

supdef::tokenizer::~tokenizer()
{
}

struct state
{
    /* const */ ::supdef::shared_ptr<const stdfs::path> filename;
    /* const */ std::u32string::const_iterator start;
    /* const */ std::u32string::const_iterator end;
    std::u32string::const_iterator next;
    size_t line;
    size_t col;
#if 0
    supdef::token_kind kind;
    std::optional<supdef::keyword_kind> keyword;
    std::optional<std::u32string> data;
#else
    ::supdef::token tokret;
#endif

    auto advance() -> decltype(next)
    {
        if (next == end)
            return next;
        decltype(next) prev = next;
        if (*next == U'\n')
        {
            ++line;
            col = 1;
        }
        else
            ++col;
        ++next;
        return prev;
    }

    auto advance(size_t n) -> decltype(next)
    {
        assert(n != 0);
        auto prev = next;
        for (size_t i = 0; i < n; ++i)
            advance();
        return prev;
    }
};

using maybe_state = std::optional<std::reference_wrapper<state>>;

namespace
{
    static maybe_state match_punct_token(state& s)
    {
        switch (*s.next)
        {
#undef  TOKEN_CASE
#define TOKEN_CASE(c, k)                \
    case U ## c: {                      \
        s.tokret = {                    \
            .loc = {                    \
                .filename = s.filename, \
                .line = s.line,         \
                .column = s.col,        \
                .infile_offset =        \
                    __CKD_DISTANCE(     \
                        infile_offset,  \
                        s.start,        \
                        s.next          \
                    ),                  \
                .toksize = 1            \
            },                          \
            .data = std::u32string(     \
                1,                      \
                U ## c                  \
            ),                          \
            .keyword = std::nullopt,    \
            .kind =                     \
                supdef::token_kind::k   \
        };                              \
        s.advance();                    \
        return std::ref(s);             \
    } break

            TOKEN_CASE('(', lparen);
            TOKEN_CASE(')', rparen);
            TOKEN_CASE('{', lbrace);
            TOKEN_CASE('}', rbrace);
            TOKEN_CASE('[', lbracket);
            TOKEN_CASE(']', rbracket);
            TOKEN_CASE('<', langle);
            TOKEN_CASE('>', rangle);
            TOKEN_CASE(',', comma);
            TOKEN_CASE(';', semicolon);
            TOKEN_CASE(':', colon);
            TOKEN_CASE('=', equals);
            TOKEN_CASE('+', plus);
            TOKEN_CASE('-', minus);
            TOKEN_CASE('*', asterisk);
            TOKEN_CASE('/', slash);
            TOKEN_CASE('%', percent);
            TOKEN_CASE('&', ampersand);
            TOKEN_CASE('|', pipe);
            TOKEN_CASE('^', caret);
            TOKEN_CASE('!', exclamation);
            TOKEN_CASE('?', question);
            TOKEN_CASE('$', dollar);
            TOKEN_CASE('@', at);
            TOKEN_CASE('~', tilde);
            TOKEN_CASE('.', period);
            TOKEN_CASE('#', hash);
            default:
                return std::nullopt;
#undef TOKEN_CASE
        }
    }

    static maybe_state match_horizws(state& s)
    {
        if (supdef::unicat::is_blank(*s.next))
        {
            s.tokret = {
                .loc = {
                    .filename = s.filename,
                    .line = s.line,
                    .column = s.col,
                    .infile_offset = __CKD_DISTANCE(infile_offset, s.start, s.next),
                    .toksize = 1
                },
                .data = std::u32string(1, *s.next),
                .keyword = std::nullopt,
                .kind = supdef::token_kind::horizontal_whitespace
            };
            s.advance();
            return std::ref(s);
        }
        return std::nullopt;
    }

    // TODO: for number literals, add support for scientific notation
    static maybe_state match_lit(state& s)
    {
        auto col_start  = s.col,
             line_start = s.line;
#if 0
        if (*s.next == U'\'')
        {
            char32_t buffer[2];
            size_t count;
            s.advance();
            if (*s.next == U'\\')
            {
                count = 2;
                buffer[0] = *s.advance();
                buffer[1] = *s.advance();
            }
            else
            {
                count = 1;
                buffer[0] = *s.advance();
            }
            assert(*s.next == U'\'');

            s.advance();
            s.tokret = {
                .loc = {
                    .filename = s.filename,
                    .line = line_start,
                    .column = col_start,
                    .infile_offset = __CKD_DISTANCE(infile_offset, s.start, s.next) - count - 2,
                    .toksize = count + 2
                },
                .data = std::u32string(buffer, count),
                .keyword = std::nullopt,
                .kind = supdef::token_kind::char_literal
            };
            return std::ref(s);
        }

        if (*s.next == U'"')
#else
        if (*s.next == U'"' || *s.next == U'\'')
#endif
        {
            supdef::token_kind lit_style = *s.next == U'"' ?
                supdef::token_kind::string_literal         :
                supdef::token_kind::char_literal;
            size_t count = 0;
            std::u32string buffer;
            s.advance();
            using namespace std::string_view_literals;
            // TODO: handle unterminated string (and char) literals
            while (*s.next != 
                        (lit_style == supdef::token_kind::string_literal ?
                                                                    U'"' :
                                                                    U'\''))
            {
                if (*s.next == U'\\')
                {
                    buffer.push_back(*s.advance());
                    buffer.push_back(*s.advance());
                    count += 2;
                }
                else
                {
                    buffer.push_back(*s.advance());
                    ++count;
                }
            }

            s.advance();
            s.tokret = {
                .loc = {
                    .filename = s.filename,
                    .line = line_start,
                    .column = col_start,
                    .infile_offset = __CKD_DISTANCE(infile_offset, s.start, s.next) - count - 2,
                    .toksize = count + 2
                },
                .data = buffer,
                .keyword = std::nullopt,
                .kind = lit_style
            };
            return std::ref(s);
        }

        if (std::distance(s.next, s.end) >= 4 && std::u32string_view{s.next, s.next + 4} == U"true")
        {
            s.advance(4);
            s.tokret = {
                .loc = {
                    .filename = s.filename,
                    .line = line_start,
                    .column = col_start,
                    .infile_offset = __CKD_DISTANCE(infile_offset, s.start, s.next) - 4,
                    .toksize = 4
                },
                .data = std::u32string(U"true"),
                .keyword = std::nullopt,
                .kind = supdef::token_kind::boolean_literal
            };
            return std::ref(s);
        }

        if (std::distance(s.next, s.end) >= 5 && std::u32string_view{s.next, s.next + 5} == U"false")
        {
            s.advance(5);
            s.tokret = {
                .loc = {
                    .filename = s.filename,
                    .line = line_start,
                    .column = col_start,
                    .infile_offset = __CKD_DISTANCE(infile_offset, s.start, s.next) - 5,
                    .toksize = 5
                },
                .data = std::u32string(U"false"),
                .keyword = std::nullopt,
                .kind = supdef::token_kind::boolean_literal
            };
            return std::ref(s);
        }

        if (*s.next == U'0' && s.next + 1 != s.end)
        {
            auto cpy = s.next;
            if (s.next[1] == U'x')
            {
                s.advance(2);
                while (s.next != s.end && supdef::unicat::is_xdigit(*s.next))
                    s.advance();

                s.tokret = {
                    .loc = {
                        .filename = s.filename,
                        .line = line_start,
                        .column = col_start,
                        .infile_offset = __CKD_DISTANCE(infile_offset, s.start, cpy),
                        .toksize = __CKD_DISTANCE(toksize, cpy, s.next)
                    },
                    .data = std::u32string(cpy + 2, s.next),
                    .keyword = std::nullopt,
                    .kind = supdef::token_kind::hex_integer_literal
                };
                return std::ref(s);
            }
            else if (s.next[1] == U'b')
            {
                s.advance(2);
                while (s.next != s.end && supdef::unicat::is_bindigit(*s.next))
                    s.advance();

                s.tokret = {
                    .loc = {
                        .filename = s.filename,
                        .line = line_start,
                        .column = col_start,
                        .infile_offset = __CKD_DISTANCE(infile_offset, s.start, cpy),
                        .toksize = __CKD_DISTANCE(toksize, cpy, s.next)
                    },
                    .data = std::u32string(cpy + 2, s.next),
                    .keyword = std::nullopt,
                    .kind = supdef::token_kind::binary_integer_literal
                };
                return std::ref(s);
            }
            else
            {
                s.advance();
                while (s.next != s.end && supdef::unicat::is_odigit(*s.next))
                    s.advance();

                if (std::distance(cpy, s.next) > 1)
                {
                    s.tokret = {
                        .loc = {
                            .filename = s.filename,
                            .line = line_start,
                            .column = col_start,
                            .infile_offset = __CKD_DISTANCE(infile_offset, s.start, cpy),
                            .toksize = __CKD_DISTANCE(toksize, cpy, s.next)
                        },
                        .data = std::u32string(cpy + 1, s.next),
                        .keyword = std::nullopt,
                        .kind = supdef::token_kind::octal_integer_literal
                    };
                    return std::ref(s);
                }

                s.tokret = {
                    .loc = {
                        .filename = s.filename,
                        .line = line_start,
                        .column = col_start,
                        .infile_offset = __CKD_DISTANCE(infile_offset, s.start, cpy),
                        .toksize = 1
                    },
                    .data = std::u32string(1, U'0'),
                    .keyword = std::nullopt,
                    .kind = supdef::token_kind::integer_literal
                };
                return std::ref(s);
            }
        }

        if (supdef::unicat::is_digit(*s.next))
        {
            auto cpy = s.next;

            while (s.next != s.end && supdef::unicat::is_digit(*s.next))
                s.advance();

            if (s.next != s.end && *s.next == U'.')
            {
                s.advance();
                while (s.next != s.end && supdef::unicat::is_digit(*s.next))
                    s.advance();

                if (s.next != s.end && (*s.next == U'e' || *s.next == U'E'))
                {
                    s.advance();
                    if (s.next != s.end && (*s.next == U'+' || *s.next == U'-'))
                        s.advance();
                    while (s.next != s.end && supdef::unicat::is_digit(*s.next))
                        s.advance();
                }

                s.tokret = {
                    .loc = {
                        .filename = s.filename,
                        .line = line_start,
                        .column = col_start,
                        .infile_offset = __CKD_DISTANCE(infile_offset, s.start, cpy),
                        .toksize = __CKD_DISTANCE(toksize, cpy, s.next)
                    },
                    .data = std::u32string(cpy, s.next),
                    .keyword = std::nullopt,
                    .kind = supdef::token_kind::floating_literal
                };
                return std::ref(s);
            }

            s.tokret = {
                .loc = {
                    .filename = s.filename,
                    .line = line_start,
                    .column = col_start,
                    .infile_offset = __CKD_DISTANCE(infile_offset, s.start, cpy),
                    .toksize = __CKD_DISTANCE(toksize, cpy, s.next)
                },
                .data = std::u32string(cpy, s.next),
                .keyword = std::nullopt,
                .kind = supdef::token_kind::integer_literal
            };
            return std::ref(s);
        }

        return std::nullopt;
    }

    static maybe_state match_keyword(state& s)
    {
#undef  __KWD_KIND_LIST
#define __KWD_KIND_LIST             \
    import, embed, dump,            \
                                    \
    supdef, runnable,               \
    begin, end,                     \
                                    \
    set, unset, pragma,             \
    global, defer,                  \
                                    \
    if_, elseif, else_, endif,      \
    for_, endfor,                   \
    foreach, foreachi, endforeach,  \
                                    \
    join, split,                    \
    str, unstr,                     \
    len, math,                      \
    raw

#undef  __KWD_LIST
#define __KWD_LIST(sep) \
    import sep()        \
    embed sep()         \
    dump sep()          \
    supdef sep()        \
    runnable sep()      \
    begin sep()         \
    end sep()           \
    set sep()           \
    unset sep()         \
    pragma sep()        \
    global sep()        \
    defer sep()         \
    if sep()            \
    elseif sep()        \
    else sep()          \
    endif sep()         \
    for sep()           \
    endfor sep()        \
    foreach sep()       \
    foreachi sep()      \
    endforeach sep()    \
    join sep()          \
    split sep()         \
    str sep()           \
    unstr sep()         \
    len sep()           \
    math sep()          \
    raw

#undef  __KWD_FOREACH
#define __KWD_FOREACH(macro, ...)   \
    BOOST_PP_SEQ_FOR_EACH_I(        \
        macro,                      \
        (__VA_ARGS__),              \
        BOOST_PP_VARIADIC_TO_SEQ(   \
            __KWD_LIST(             \
                BOOST_PP_COMMA      \
            )                       \
        )                           \
    )

#undef  __KWD_USTRINGIZE_EACH
#define __KWD_USTRINGIZE_EACH(r, data, i, elem) BOOST_PP_COMMA_IF(BOOST_PP_BOOL(i)) BOOST_PP_CAT(U, BOOST_PP_STRINGIZE(elem))

#undef  __KWD_KWSTRUCT_INIT
#define __KWD_KWSTRUCT_INIT(r, data, i, elem)           \
    BOOST_PP_COMMA_IF(BOOST_PP_BOOL(i)) {               \
        BOOST_PP_CAT(                                   \
            U, BOOST_PP_STRINGIZE(elem)                 \
        ), 0,                                           \
        ::supdef::keyword_kind::BOOST_PP_TUPLE_ELEM(    \
            i, data                                     \
        ), true                                         \
    }

        static_assert(BOOST_PP_SEQ_SIZE(BOOST_PP_VARIADIC_TO_SEQ(__KWD_LIST(BOOST_PP_COMMA))) == int(::supdef::keyword_kind::unknown), "keyword list size mismatch");
        static_assert(BOOST_PP_SEQ_SIZE(BOOST_PP_VARIADIC_TO_SEQ(__KWD_KIND_LIST)) == int(::supdef::keyword_kind::unknown), "keyword list size mismatch");

        static constexpr const std::u32string_view keywords_strs[] = {
            __KWD_FOREACH(__KWD_USTRINGIZE_EACH)
        };
        static constexpr const size_t keyword_count = sizeof(keywords_strs) / sizeof(keywords_strs[0]);
        static constexpr const size_t max_keyword_length = std::max_element(
            std::begin(keywords_strs),
            std::end(keywords_strs),
            [](const auto& a, const auto& b) { return a.size() < b.size(); }
        )->size();
        
        struct {
            const std::u32string_view kw;
            size_t matched;
            supdef::keyword_kind kw_kind;
            bool matched_so_far;
        } keywords[] = {
            __KWD_FOREACH(__KWD_KWSTRUCT_INIT, __KWD_KIND_LIST)
        };
        size_t index = 0;

        auto line_start = s.line,
             col_start  = s.col;

        auto beginning = s.next;
        while (s.next != s.end && index < max_keyword_length)
        {
            for (size_t i = 0; i < keyword_count; ++i)
            {
                if (keywords[i].matched_so_far && keywords[i].matched < keywords[i].kw.size())
                {
                    if (keywords[i].kw[keywords[i].matched] == *s.next)
                        ++keywords[i].matched;
                    else
                        keywords[i].matched_so_far = false;
                }
            }

            ++s.next;
            ++index;
        }

        for (size_t i = 0; i < keyword_count; ++i)
        {
            if (keywords[i].matched == keywords[i].kw.size() && keywords[i].matched_so_far)
            {
                size_t max_size_matched_index = i,
                       max_size_matched       = keywords[i].matched;
                for (size_t j = 0; j < keyword_count; ++j)
                {
                    if (keywords[j].matched > max_size_matched)
                    {
                        max_size_matched_index = j;
                        max_size_matched = keywords[j].matched;
                    }
                }

                s.next = beginning + max_size_matched;
                s.col += max_size_matched;
                s.tokret = {
                    .loc = {
                        .filename = s.filename,
                        .line = line_start,
                        .column = col_start,
                        .infile_offset = __CKD_DISTANCE(infile_offset, s.start, beginning),
                        .toksize = max_size_matched
                    },
                    .data = keywords[max_size_matched_index].kw.data(),
                    .keyword = keywords[max_size_matched_index].kw_kind,
                    .kind = supdef::token_kind::keyword
                };
                return std::ref(s);
            }
        }

        return std::nullopt;
    }

    static maybe_state match_identifier(state& s)
    {
        static constexpr auto is_id_start = [](char32_t c) -> bool
        {
            return supdef::unicat::is_alpha(c) || c == U'_';
        };
        static constexpr auto is_id_continue = [](char32_t c) -> bool
        {
            return supdef::unicat::is_alnum(c) || c == U'_';
        };

        const auto start = s.next;
        const auto line_start = s.line,
                   col_start  = s.col;
        if (is_id_start(*s.next))
        {
            while (s.next != s.end && is_id_continue(*s.next))
                s.advance();

            s.tokret = {
                .loc = {
                    .filename = s.filename,
                    .line = line_start,
                    .column = col_start,
                    .infile_offset = __CKD_DISTANCE(infile_offset, s.start, start),
                    .toksize = __CKD_DISTANCE(toksize, start, s.next)
                },
                .data = std::u32string(start, s.next),
                .keyword = std::nullopt,
                .kind = supdef::token_kind::identifier
            };
            return std::ref(s);
        }

        return std::nullopt;
    }

    static maybe_state match_backslash(state& s)
    {
        if (*s.next == U'\\')
        {
            s.tokret = {
                .loc = {
                    .filename = s.filename,
                    .line = s.line,
                    .column = s.col,
                    .infile_offset = __CKD_DISTANCE(infile_offset, s.start, s.next),
                    .toksize = 1
                },
                .data = std::u32string(1, U'\\'),
                .keyword = std::nullopt,
                .kind = supdef::token_kind::backslash
            };
            s.advance();
            return std::ref(s);
        }
        return std::nullopt;
    }

    static maybe_state match_newline(state& s)
    {
        if (*s.next == U'\n')
        {
            s.tokret = {
                .loc = {
                    .filename = s.filename,
                    .line = s.line,
                    .column = s.col,
                    .infile_offset = __CKD_DISTANCE(infile_offset, s.start, s.next),
                    .toksize = 1
                },
                .data = std::u32string(1, U'\n'),
                .keyword = std::nullopt,
                .kind = supdef::token_kind::newline
            };
            s.advance();
            return std::ref(s);
        }
        return std::nullopt;
    }

    static maybe_state match_comment(state& s)
    {
        const auto line_start = s.line,
                   col_start  = s.col;
        const auto start = s.next;
        if (*s.next == U'/')
        {
            if (s.next + 1 != s.end && s.next[1] == U'/')
            {
                s.advance(2);
                while (s.next != s.end && *s.next != U'\\' && *s.next != U'\n')
                    s.advance();

                s.tokret = {
                    .loc = {
                        .filename = s.filename,
                        .line = line_start,
                        .column = col_start,
                        .infile_offset = __CKD_DISTANCE(infile_offset, s.start, start),
                        .toksize = __CKD_DISTANCE(toksize, start, s.next)
                    },
                    .data = std::u32string(start, s.next),
                    .keyword = std::nullopt,
                    .kind = supdef::token_kind::inline_comment
                };
                return std::ref(s);
            }
            else if (s.next + 1 != s.end && s.next[1] == U'*')
            {
                s.advance(2);
                while (s.next + 1 != s.end && !(s.next[0] == U'*' && s.next[1] == U'/'))
                    s.advance();
                
                s.advance(2);
                s.tokret = {
                    .loc = {
                        .filename = s.filename,
                        .line = line_start,
                        .column = col_start,
                        .infile_offset = __CKD_DISTANCE(infile_offset, s.start, start),
                        .toksize = __CKD_DISTANCE(toksize, start, s.next)
                    },
                    .data = std::u32string(start, s.next),
                    .keyword = std::nullopt,
                    .kind = supdef::token_kind::multiline_comment
                };
                return std::ref(s);
            }
        }
        return std::nullopt;
    }

    SAFE_STATIC supdef::token_kind prev_token_kind = supdef::token_kind::newline;

    static maybe_state maybe_start_of_line(state& s)
    {
        if (prev_token_kind == supdef::token_kind::newline)
        {
            s.tokret = {
                .loc = {
                    .filename = s.filename,
                    .line = s.line,
                    .column = s.col,
                    .infile_offset = __CKD_DISTANCE(infile_offset, s.start, s.next),
                    .toksize = 0
                },
                .data = std::nullopt,
                .keyword = std::nullopt,
                .kind = supdef::token_kind::line_start
            };
            return std::ref(s);
        }
        return std::nullopt;
    }

    static state& extract_next_token(state& s)
    {
        constexpr auto do_return = [](state& s) -> state& {
            prev_token_kind = s.tokret.kind;
            return s;
        };

        const auto next_cpy = s.next;

        maybe_state is_line_start = maybe_start_of_line(s);
        if (is_line_start.has_value())
            return do_return(is_line_start.value());
        s.next = next_cpy;

        if (s.next == s.end)
        {
            s.tokret = {
                .loc = {
                    .filename = s.filename,
                    .line = s.line,
                    .column = s.col,
                    // let's make it 1-sized although it's not
                    .infile_offset = __CKD_DISTANCE(infile_offset, s.start, s.end) - 1,
                    // let's make it 1-sized although it's not
                    .toksize = 1
                },
                .data = std::nullopt,
                .keyword = std::nullopt,
                .kind = supdef::token_kind::eof
            };
            return do_return(s);
        }

        // must be before match_punct_token
        maybe_state is_comment = match_comment(s);
        if (is_comment.has_value())
            return do_return(is_comment.value());
        s.next = next_cpy;

        maybe_state is_known_punct = match_punct_token(s);
        if (is_known_punct.has_value())
            return do_return(is_known_punct.value());
        s.next = next_cpy;

        maybe_state is_horizws = match_horizws(s);
        if (is_horizws.has_value())
            return do_return(is_horizws.value());
        s.next = next_cpy;

        maybe_state is_newline = match_newline(s);
        if (is_newline.has_value())
            return do_return(is_newline.value());
        s.next = next_cpy;
        
        maybe_state is_lit = match_lit(s);
        if (is_lit.has_value())
            return do_return(is_lit.value());
        s.next = next_cpy;

        maybe_state is_backslash = match_backslash(s);
        if (is_backslash.has_value())
            return do_return(is_backslash.value());
        s.next = next_cpy;
    
        maybe_state is_keyword = match_keyword(s);
        if (is_keyword.has_value())
            return do_return(is_keyword.value());
        s.next = next_cpy;

        maybe_state is_ident = match_identifier(s);
        if (is_ident.has_value())
            return do_return(is_ident.value());
        s.next = next_cpy;

        s.tokret = {
            .loc = {
                .filename = s.filename,
                .line = s.line,
                .column = s.col,
                .infile_offset = __CKD_DISTANCE(infile_offset, s.start, s.next),
                .toksize = 1
            },
            .data = std::u32string(1, *s.next),
            .keyword = std::nullopt,
            .kind = supdef::token_kind::other
        };
        s.advance();
        return do_return(s);
    }
}


std::generator<supdef::token> supdef::tokenizer::tokenize(::supdef::shared_ptr<const stdfs::path> filename)
{
    auto processed_filename = filename ?: m_filename;
    if (!processed_filename)
        throw std::invalid_argument("filename is required");
    
    prev_token_kind = supdef::token_kind::newline;

    state s {
        .filename = processed_filename,
        .start = m_data.begin(),
        .end = m_data.end(),
        .next = m_data.begin(),
        .line = 1,
        .col = 1,
        .tokret = {
            .loc = {
                .filename = processed_filename,
                .line = 1,
                .column = 1,
                .infile_offset = 0,
                .toksize = 0
            },
            .data = std::nullopt,
            .keyword = std::nullopt,
            .kind = supdef::token_kind::other
        }
    };

    while (true)
    {
        s = extract_next_token(s);
        co_yield s.tokret;
        if (s.tokret.kind == supdef::token_kind::eof)
            break;
    }

    co_return;
}

std::string supdef::description_string(::supdef::token_kind kind)
{
    std::string basic_desc = translate(
        magic_enum::enum_name(kind).data(), "_", " "
    );
    std::string desc,
                prefix = "token `",
                suffix = "`";
    switch (kind)
    {
    case token_kind::eof:
        desc = "(end of file marker)";
        break;
    case token_kind::line_start:
        desc = "(start of line marker)";
        break;
    case token_kind::horizontal_whitespace:
        desc = "";
        break;
    case token_kind::newline:
        desc = "";
        break;
    case token_kind::inline_comment:
        desc = "(`//...` comment)";
        break;
    case token_kind::multiline_comment:
        desc = "(`/*...*/` comment)";
        break;
    case token_kind::char_literal:
        desc = "('...')";
        break;
    case token_kind::string_literal:
        desc = "(\"...\")";
        break;
    case token_kind::boolean_literal:
        desc = "(`true` or `false`)";
        break;
    case token_kind::integer_literal:
        desc = "(1234567890...)";
        break;
    case token_kind::hex_integer_literal:
        desc = "(0x1234567890...)";
        break;
    case token_kind::octal_integer_literal:
        desc = "(01234567...)";
        break;
    case token_kind::binary_integer_literal:
        desc = "(0b01010101...)";
        break;
    case token_kind::floating_literal:
        desc = "(123.456e-789...)";
        break;
    case token_kind::keyword:
        desc = "";
        break;
    case token_kind::identifier:
        desc = "";
        break;
    case token_kind::backslash:
        desc = "(`\\`)";
        break;
    case token_kind::lparen:
        desc = "(`(`)";
        break;
    case token_kind::rparen:
        desc = "(`)`)";
        break;
    case token_kind::lbrace:
        desc = "(`{`)";
        break;
    case token_kind::rbrace:
        desc = "(`}`)";
        break;
    case token_kind::lbracket:
        desc = "(`[`)";
        break;
    case token_kind::rbracket:
        desc = "(`]`)";
        break;
    case token_kind::langle:
        desc = "(`<`)";
        break;
    case token_kind::rangle:
        desc = "(`>`)";
        break;
    case token_kind::comma:
        desc = "(`,`)";
        break;
    case token_kind::semicolon:
        desc = "(`;`)";
        break;
    case token_kind::colon:
        desc = "(`:`)";
        break;
    case token_kind::equals:
        desc = "(`=`)";
        break;
    case token_kind::plus:
        desc = "(`+`)";
        break;
    case token_kind::minus:
        desc = "(`-`)";
        break;
    case token_kind::asterisk:
        desc = "(`*`)";
        break;
    case token_kind::slash:
        desc = "(`/`)";
        break;
    case token_kind::percent:
        desc = "(`%`)";
        break;
    case token_kind::ampersand:
        desc = "(`&`)";
        break;
    case token_kind::pipe:
        desc = "(`|`)";
        break;
    case token_kind::caret:
        desc = "(`^`)";
        break;
    case token_kind::exclamation:
        desc = "(`!`)";
        break;
    case token_kind::question:
        desc = "(`?`)";
        break;
    case token_kind::dollar:
        desc = "(`$`)";
        break;
    case token_kind::at:
        desc = "(`@`)";
        break;
    case token_kind::tilde:
        desc = "(`~`)";
        break;
    case token_kind::period:
        desc = "(`.`)";
        break;
    case token_kind::hash:
        desc = "(`#`)";
        break;
    case token_kind::other:
        desc = "";
        break;
    default:
        desc = "(unknown)";
        break;
    }
    if (!desc.empty())
        suffix += " ";
    return prefix + basic_desc + suffix + desc; 
}
std::string supdef::description_string(::supdef::keyword_kind kind)
{
    std::string desc = strip(magic_enum::enum_name(kind).data(), "_\t\n\r\v\f");
    if (desc == "unknown")
        return "unknown keyword";
    return "keyword `" + desc + "`";
}