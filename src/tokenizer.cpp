/* class tokenizer
{
public:
    tokenizer(const std::u32string& data);
    ~tokenizer();

    std::generator<token> tokenize();

private:
    std::u32string m_data;
}; */

#include <tokenizer.hpp>
#include <unicode.hpp>

#include <simdutf.h>

#include <boost/preprocessor.hpp>

namespace
{
    static std::string format(const std::u32string& str, size_t start = 0, size_t end = std::u32string::npos)
    {
        const auto& dataptr = str.data();
        const auto& datalen = str.size();
        const auto& datastart = dataptr + start;

        if (end == std::u32string::npos)
            end = datalen;

        size_t required_length = simdutf::utf8_length_from_utf32(datastart, end - start);
        std::unique_ptr<char[]> buf(new char[required_length]);
        size_t result = simdutf::convert_valid_utf32_to_utf8(datastart, end - start, buf.get());
        if (result == 0)
            throw std::runtime_error("failed to convert utf32 to utf8");

        return std::string(buf.get(), result);
    }

    static inline std::string format(const std::u32string::const_iterator& start, const std::u32string::const_iterator& end)
    {
        return format(std::u32string(start, end));
    }

    static inline std::string format(const std::u32string::iterator& start, const std::u32string::iterator& end)
    {
        return format(std::u32string(start, end));
    }
}

supdef::tokenizer::tokenizer(const std::u32string& data)
    : m_data(data)
{
}

supdef::tokenizer::~tokenizer()
{
}

struct state
{
    /* const */ std::u32string::const_iterator start;
    /* const */ std::u32string::const_iterator end;
    std::u32string::const_iterator next;
    supdef::token_kind kind;
    std::optional<supdef::keyword_kind> keyword;
    std::optional<std::u32string> data;
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
        s.kind = supdef::token_kind::k; \
        s.data = std::u32string(        \
            1,                          \
            U ## c                      \
        );                              \
        s.keyword = std::nullopt;       \
        ++s.next;                       \
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
            default:
                return std::nullopt;
#undef TOKEN_CASE
        }
    }

    static maybe_state match_horizws(state& s)
    {
        bool (*funcptr)(char32_t) = &supdef::unicat::is_blank;
        if (funcptr(*s.next))
        {
            s.kind = supdef::token_kind::horizontal_whitespace;
            s.data = std::u32string(1, *s.next);
            s.keyword = std::nullopt;
            ++s.next;
            return std::ref(s);
        }
        return std::nullopt;
    }

    static maybe_state match_lit(state& s)
    {
        if (*s.next == U'\'')
        {
            char32_t buffer[2];
            s.next++;
            if (*s.next == U'\\')
            {
                buffer[0] = *s.next++;
                buffer[1] = *s.next++;
                if (*s.next != U'\'')
                    std::unreachable(); // already handled when removing comments

                s.next++;
                s.kind = supdef::token_kind::char_literal;
                s.data = std::u32string(buffer, 2);
                s.keyword = std::nullopt;
                return std::ref(s);
            }
            else
            {
                buffer[0] = *s.next++;
                if (*s.next != U'\'')
                    std::unreachable(); // already handled when removing comments

                s.next++;
                s.kind = supdef::token_kind::char_literal;
                s.data = std::u32string(buffer, 1);
                s.keyword = std::nullopt;
                return std::ref(s);
            }
        }

        if (*s.next == U'"')
        {
            std::u32string buffer;
            s.next++;
            while (*s.next != U'"')
            {
                if (*s.next == U'\\')
                {
                    buffer.push_back(*s.next++);
                    buffer.push_back(*s.next++);
                }
                else
                    buffer.push_back(*s.next++);
            }

            s.next++;
            s.kind = supdef::token_kind::string_literal;
            s.data = buffer;
            s.keyword = std::nullopt;
            return std::ref(s);
        }

        if (*s.next == U'0' && s.next + 1 != s.end)
        {
            auto cpy = s.next;
            if (s.next[1] == U'x')
            {
                s.next += 2;
                while (s.next != s.end && supdef::unicat::is_xdigit(*s.next))
                    ++s.next;

                s.kind = supdef::token_kind::hex_integer_literal;
                s.data = std::u32string(cpy + 2, s.next);
                s.keyword = std::nullopt;
                return std::ref(s);
            }
            else if (s.next[1] == U'b')
            {
                s.next += 2;
                while (s.next != s.end && (*s.next == U'0' || *s.next == U'1'))
                    ++s.next;

                s.kind = supdef::token_kind::binary_integer_literal;
                s.data = std::u32string(cpy + 2, s.next);
                s.keyword = std::nullopt;
                return std::ref(s);
            }
            else
            {
                while (s.next != s.end && supdef::unicat::is_odigit(*s.next))
                    ++s.next;

                if (cpy != s.next - 1)
                {
                    s.kind = supdef::token_kind::octal_integer_literal;
                    s.data = std::u32string(cpy, s.next);
                    s.keyword = std::nullopt;
                    return std::ref(s);
                }
                
                s.kind = supdef::token_kind::integer_literal;
                s.data = std::u32string(1, U'0');
                s.keyword = std::nullopt;
                return std::ref(s);
            }
        }

        if (supdef::unicat::is_digit(*s.next))
        {
            auto cpy = s.next;

            while (s.next != s.end && supdef::unicat::is_digit(*s.next))
                ++s.next;

            if (s.next != s.end && *s.next == U'.')
            {
                ++s.next;
                while (s.next != s.end && supdef::unicat::is_digit(*s.next))
                    ++s.next;

                if (s.next != s.end && (*s.next == U'e' || *s.next == U'E'))
                {
                    ++s.next;
                    if (s.next != s.end && (*s.next == U'+' || *s.next == U'-'))
                        ++s.next;
                    while (s.next != s.end && supdef::unicat::is_digit(*s.next))
                        ++s.next;
                }

                s.kind = supdef::token_kind::floating_literal;
                s.data = std::u32string(cpy, s.next);
                s.keyword = std::nullopt;
                return std::ref(s);
            }

            s.kind = supdef::token_kind::integer_literal;
            s.data = std::u32string(cpy, s.next);
            s.keyword = std::nullopt;
            return std::ref(s);
        }

        return std::nullopt;
    }

    static maybe_state match_keyword(state& s)
    {
#undef  __KWD_KIND_LIST
#define __KWD_KIND_LIST         \
    supdef, import, runnable,   \
    embed, dump, let, if_,      \
    elseif, else_, endif,       \
    for_, end, join, split,     \
    str, unstr, len, math, begin

#undef  __KWD_LIST
#define __KWD_LIST(sep) \
    supdef sep()        \
    import sep()        \
    runnable sep()      \
    embed sep()         \
    dump sep()          \
    let sep()           \
    if sep()            \
    elseif sep()        \
    else sep()          \
    endif sep()         \
    for sep()           \
    end sep()           \
    join sep()          \
    split sep()         \
    str sep()           \
    unstr sep()         \
    len sep()           \
    math sep()          \
    begin

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
            const std::u32string kw;
            size_t matched;
            supdef::keyword_kind kw_kind;
            bool matched_so_far;
        } keywords[] = {
            __KWD_FOREACH(__KWD_KWSTRUCT_INIT, __KWD_KIND_LIST)
        };
        size_t index = 0;

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
                s.kind = supdef::token_kind::keyword;
                s.data = keywords[max_size_matched_index].kw;
                s.keyword = keywords[max_size_matched_index].kw_kind;
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
        if (is_id_start(*s.next))
        {
            while (s.next != s.end && is_id_continue(*s.next))
                ++s.next;

            s.kind = supdef::token_kind::identifier;
            s.data = std::u32string(start, s.next);
            s.keyword = std::nullopt;
            return std::ref(s);
        }

        return std::nullopt;
    }

    static state& extract_next_token(state& s)
    {
        const auto next_cpy = s.next;

        if (s.next == s.end)
        {
            s.kind = supdef::token_kind::eof;
            s.data = std::nullopt;
            s.keyword = std::nullopt;
            return s;
        }

        maybe_state is_known_punct = match_punct_token(s);
        if (is_known_punct.has_value())
            return is_known_punct.value();
        s.next = next_cpy;

        maybe_state is_horizws = match_horizws(s);
        if (is_horizws.has_value())
            return is_horizws.value();
        s.next = next_cpy;
        
        if (*s.next == U'\n')
        {
            s.kind = supdef::token_kind::newline;
            s.data = std::u32string(1, U'\n');
            s.keyword = std::nullopt;
            ++s.next;
            return s;
        }

        maybe_state is_lit = match_lit(s);
        if (is_lit.has_value())
            return is_lit.value();
        s.next = next_cpy;
    
        maybe_state is_keyword = match_keyword(s);
        if (is_keyword.has_value())
            return is_keyword.value();
        s.next = next_cpy;

        maybe_state is_ident = match_identifier(s);
        if (is_ident.has_value())
            return is_ident.value();
        s.next = next_cpy;

        s.kind = supdef::token_kind::other;
        s.data = std::u32string(1, *s.next);
        s.keyword = std::nullopt;
        ++s.next;
        return s;
    }
}


std::generator<supdef::token> supdef::tokenizer::tokenize()
{
    state s{
        .start = m_data.begin(),
        .end = m_data.end(),
        .next = m_data.begin(),
        .kind = supdef::token_kind::other,
        .keyword = std::nullopt,
        .data = std::nullopt
    };

    while (true)
    {
        s.kind = token_kind::other;
        s.keyword = std::nullopt;
        s.data = std::nullopt;
        s = extract_next_token(s);
        co_yield token{ s.data, s.keyword, s.kind };
        if (s.kind == token_kind::eof)
            break;
    }

    co_return;
}