#include <ast.hpp>
#include <parser.hpp>
#include <tokenizer.hpp>
#include <types.hpp>
#include <impl/parsing-utils.tpp>

#include <bits/stdc++.h>

supdef::ast::builder::builder(const std::list<::supdef::token>& tokens)
    : m_tokens(std::addressof(tokens))
{
}

supdef::ast::builder::builder(const std::vector<::supdef::token>& tokens)
    : m_tokens(std::addressof(tokens))
{
}

struct tok_info
{
    std::optional<supdef::token_kind> tkind = std::nullopt;
    std::optional<supdef::keyword_kind> kkind = std::nullopt;
    std::optional<std::u32string> data = std::nullopt;
    bool from_token = false;

    tok_info(const ::supdef::token& tok)
        : tkind(tok.kind)
        , kkind(tok.keyword)
        , data(tok.data)
        , from_token(true)
    {
    }
    

    std::string to_string() const
    {
        using namespace std::string_literals;
        if (this->kkind.has_value())
        {
            std::string kwstring = this->data.has_value() && !this->from_token
                                 ? "keyword `" + ::supdef::printer::format(this->data.value()) + "`"
                                 : ::supdef::description_string(this->kkind.value());
            return kwstring;
        }
        else if (this->tkind.has_value())
        {
            std::string tkstring = this->data.has_value() && !this->from_token
                                 ? "token `" + ::supdef::printer::format(this->data.value()) + "`"
                                 : ::supdef::description_string(this->tkind.value());
            return tkstring;
        }
        else if (this->data.has_value())
            return "`" + ::supdef::printer::format(this->data.value()) + "`";
        else
            return "any token";
    }
};

static inline bool match(
    const supdef::token& tok,
    const tok_info& info
) {
    if (info.tkind.has_value() && tok.kind != info.tkind.value())
        return false;
    if (info.kkind.has_value() && tok.keyword != info.kkind.value())
        return false;
    if (info.data.has_value() && tok.data != info.data.value())
        return false;
    return true;
}

static inline supdef::ast::parse_error mk_parse_error(
    const tok_info& expected,
    const supdef::token& got,
    const supdef::token_loc* loc = nullptr
) {
    using namespace std::string_literals;
    return supdef::ast::parse_error(
        loc ? *loc : got.loc,
        "expected "s + expected.to_string() + ", got " +
                  tok_info(got).to_string() + " instead"
    );
}

static inline auto next_token(
    points_to_token_and_bidir auto& it,
    const points_to_token_and_bidir auto& end,
    size_t count = 1
) -> std::remove_cvref_t<decltype(it)>
{
    if (it == end)
        return end;
    
    typedef std::remove_cvref_t<decltype(it)> iter_type;
    iter_type cpy = it;

    std::advance(it, count);

    return cpy;
}

static inline bool accept_token(
    points_to_token_and_bidir auto& it,
    const points_to_token_and_bidir auto& end,
    const tok_info& to_match,
    bool skip_ws = true,
    size_t min_ws = 1,
    bool skip_newlines = false
) {
    if (it == end)
        return false;

    typedef std::remove_cvref_t<decltype(it)> iter_type;
    iter_type cpy = it;

    if (skip_ws)
    {
#if 0
        size_t skipped = skipws(it, end, skip_newlines);
#else
        size_t skipped = skip_while(
            it, end, [=](const ::supdef::token& tok) -> bool {
                return !match(tok, to_match) &&
                       (tok.kind == ::supdef::token_kind::horizontal_whitespace ||
                           (skip_newlines && tok.kind == ::supdef::token_kind::newline));
            }
        );
#endif
        if (min_ws && skipped < min_ws)
            goto fail;
    }

    if (match(*it, to_match))
    {
        next_token(it, end);
        return true;
    }

fail:
    it = cpy;
    return false;
}

static inline ::supdef::token expect_token(
    points_to_token_and_bidir auto& it,
    const points_to_token_and_bidir auto& end,
    const tok_info& to_match,
    bool skip_ws = true,
    size_t min_ws = 1,
    bool skip_newlines = false
) {
    using namespace std::string_literals;
    if (!accept_token(it, end, to_match, skip_ws, min_ws, skip_newlines))
        // TODO: implement backtracking, to be able to continue parsing
        //       after an error has occurred
        throw mk_parse_error(to_match, *it);
    return *it;
}

#undef  ANYWS
#undef  HORIZWS
#define ANYWS(...) true, (__VA_ARGS__), true
#define HORIZWS(...) true, (__VA_ARGS__), false

static inline decltype(auto) get_prev_token(
    points_to_token_and_bidir auto& it,
    const points_to_token_and_bidir auto& begin
) {
    return stdranges::prev(it, 1, begin);
}

static std::expected<supdef::ast::shared_node, supdef::ast::parse_error>
parse_top_level(
    points_to_token_and_bidir auto& it,
    const points_to_token_and_bidir auto& begin,
    const points_to_token_and_bidir auto& end
) {
    using namespace supdef;
    try
    {
        if (accept_token(it, end, { .tkind = token_kind::at }, ANYWS(0)))
        {
            // test if the "at" is at the start of the line
            auto prev = get_prev_token(it, begin);
            if (at_start_of_line(it, begin))
            {
                // parse a directive
            }
            else
            {
                // parse a builtin function call (e.g. `@len()` etc.)
            }
        }
    }
    catch (const supdef::ast::parse_error& e)
    {
        return std::unexpected(e);
    }
    catch (...)
    {
        throw;
    }
    return nullptr;
}

std::generator<std::expected<supdef::ast::shared_node, supdef::ast::parse_error>>
supdef::ast::builder::build()
{
    if (std::holds_alternative<const std::list<::supdef::token>*>(m_tokens))
    {
        typedef std::list<::supdef::token>::const_iterator const_iterator;
        const_iterator it = std::get<const std::list<::supdef::token>*>(m_tokens)->cbegin();
        const const_iterator end = std::get<const std::list<::supdef::token>*>(m_tokens)->cend();
        while (it != end)
            co_yield parse_top_level(
                it,
                std::get<const std::list<::supdef::token>*>(m_tokens)->cbegin(),
                std::get<const std::list<::supdef::token>*>(m_tokens)->cend()
            );
    }
    else if (std::holds_alternative<const std::vector<::supdef::token>*>(m_tokens))
    {
        typedef std::vector<::supdef::token>::const_iterator const_iterator;
        const_iterator it = std::get<const std::vector<::supdef::token>*>(m_tokens)->cbegin();
        const const_iterator end = std::get<const std::vector<::supdef::token>*>(m_tokens)->cend();
        while (it != end)
            co_yield parse_top_level(
                it,
                std::get<const std::vector<::supdef::token>*>(m_tokens)->cbegin(),
                std::get<const std::vector<::supdef::token>*>(m_tokens)->cend()
            );
    }
    else
        throw std::runtime_error("invalid variant index");
    co_return;
}