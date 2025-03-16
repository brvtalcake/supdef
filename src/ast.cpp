#include <ast.hpp>
#include <parser.hpp>
#include <tokenizer.hpp>
#include <types.hpp>
#include <impl/parsing-utils.tpp>

#include <bits/stdc++.h>

#include <boost/mpl/for_each.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/transform.hpp>

#include <boost/hana.hpp>

#include <boost/hof.hpp>

#include <boost/mp11.hpp>

namespace hana = boost::hana;
namespace mp11 = boost::mp11;
namespace hof = boost::hof;

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

    //tok_info(const ::supdef::token& tok)
    //    : tkind(tok.kind)
    //    , kkind(tok.keyword)
    //    , data(tok.data)
    //    , from_token(true)
    //{
    //}
    
    static constexpr tok_info from(const ::supdef::token& tok);

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

/* static_assert(std::is_trivial_v<tok_info>); */
static_assert(std::is_standard_layout_v<tok_info>);

constexpr tok_info tok_info::from(const ::supdef::token& tok)
{
    return tok_info{
        .tkind = tok.kind,
        .kkind = tok.keyword,
        .data = tok.data,
        .from_token = true
    };
}

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
    std::string&& msg,
    const tok_info& expected,
    const supdef::token& got,
    const supdef::token_loc* loc = nullptr
) {
    using namespace std::string_literals;
    return supdef::ast::parse_error(
        loc ? *loc : got.loc,
        "expected "s + expected.to_string() + ", got "   +
                  tok_info::from(got).to_string() + " instead" +
                  (msg.empty() ? "" : ": " + std::move(msg))
    );
}

static_assert(std::bool_constant<stdranges::contiguous_range<std::initializer_list<tok_info>>>::value);

static inline supdef::ast::parse_error mk_parse_error(
    std::string&& msg,
    std::initializer_list<tok_info> expectations,
    const supdef::token& got,
    const supdef::token_loc* loc = nullptr
) {
    using namespace std::string_literals;
    
    if (expectations.size() == 1)
        return mk_parse_error(std::move(msg), *expectations.begin(), got, loc);
    else if (expectations.size() > 1)
    {
        std::string expected_str = "expected one of { " + expectations.begin()->to_string();
        for (const auto& it : expectations | stdviews::drop(1) | ::supdef::drop_last(1))
        {
            expected_str += ", ";
            expected_str += it.to_string();
        }
        expected_str += " or ";
        expected_str += (expectations.end() - 1)->to_string();
        expected_str += " }, got " + tok_info::from(got).to_string() + " instead";
        return supdef::ast::parse_error(
            loc ? *loc : got.loc,
            expected_str + (msg.empty() ? "" : ": " + std::move(msg))
        );
    }
    else
    {
        return supdef::ast::parse_error(
            loc ? *loc : got.loc,
            "expected any token, got " + tok_info::from(got).to_string() + " instead"
        );
    }
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
#if 1
        return true;
#else
    {
        next_token(it, end);
        return true;
    }
#endif

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
    bool skip_newlines = false,
    std::string&& msg = ""
) {
    if (!accept_token(it, end, to_match, skip_ws, min_ws, skip_newlines))
        // TODO: implement backtracking, to be able to continue parsing
        //       after an error has occurred
        throw mk_parse_error(std::move(msg), to_match, *it);
    return *it;
}

static inline ::supdef::token expect_token_alt(
    points_to_token_and_bidir auto& it,
    const points_to_token_and_bidir auto& end,
    const tok_info& to_match,
    std::initializer_list<tok_info> all_possibilities,
    bool skip_ws = true,
    size_t min_ws = 1,
    bool skip_newlines = false,
    std::string&& msg = ""
) {
    if (!accept_token(it, end, to_match, skip_ws, min_ws, skip_newlines))
        // TODO: implement backtracking, to be able to continue parsing
        //       after an error has occurred
        throw mk_parse_error(std::move(msg), all_possibilities, *it);
    return *it;
}

#undef  ANYWS
#undef  HORIZWS
#define ANYWS(...) true, (__VA_ARGS__), true
#define HORIZWS(...) true, (__VA_ARGS__), false

static inline decltype(auto) get_prev_token(
    points_to_token_and_bidir auto it,
    const points_to_token_and_bidir auto& begin
) {
    return stdranges::prev(it, 1, begin);
}

template <typename NodeT>
using parse_result = std::expected<supdef::shared_ptr<NodeT>, supdef::ast::parse_error>;

static parse_result<supdef::ast::boolean_node>
parse_boolean(
    points_to_token_and_bidir auto& it,
    const points_to_token_and_bidir auto& begin,
    const points_to_token_and_bidir auto& end,
    bool allow_non_constant = false,
    bool allow_coercion = false
);
static parse_result<supdef::ast::integer_node>
parse_integer(
    points_to_token_and_bidir auto& it,
    const points_to_token_and_bidir auto& begin,
    const points_to_token_and_bidir auto& end,
    bool allow_non_constant = false,
    bool allow_coercion = false
);
static parse_result<supdef::ast::floating_node>
parse_floating(
    points_to_token_and_bidir auto& it,
    const points_to_token_and_bidir auto& begin,
    const points_to_token_and_bidir auto& end,
    bool allow_non_constant = false,
    bool allow_coercion = false
);

static supdef::shared_ptr<supdef::ast::dump_node>
parse_dump(
    points_to_token_and_bidir auto& it,
    const points_to_token_and_bidir auto& begin,
    const points_to_token_and_bidir auto& end
);
static supdef::shared_ptr<supdef::ast::embed_node>
parse_embed(
    points_to_token_and_bidir auto& it,
    const points_to_token_and_bidir auto& begin,
    const points_to_token_and_bidir auto& end
);
static supdef::shared_ptr<supdef::ast::supdef_node>
parse_supdef(
    points_to_token_and_bidir auto& it,
    const points_to_token_and_bidir auto& begin,
    const points_to_token_and_bidir auto& end
);
static supdef::shared_ptr<supdef::ast::runnable_node>
parse_runnable(
    points_to_token_and_bidir auto& it,
    const points_to_token_and_bidir auto& begin,
    const points_to_token_and_bidir auto& end
);
static supdef::shared_ptr<supdef::ast::import_node>
parse_import(
    points_to_token_and_bidir auto& it,
    const points_to_token_and_bidir auto& begin,
    const points_to_token_and_bidir auto& end
);

inline constexpr auto expression_parsers_table = hana::make_map(
    hana::make_pair(
        supdef::ast::node::kind::boolean,
        hana::just(BOOST_HOF_LIFT(parse_boolean))
    ),
    hana::make_pair(
        supdef::ast::node::kind::integer,
        hana::just(BOOST_HOF_LIFT(parse_integer))
    ),
    hana::make_pair(
        supdef::ast::node::kind::floating,
        hana::just(BOOST_HOF_LIFT(parse_floating))
    )
);

inline constexpr auto directive_parsers_table = hana::make_tuple(
    hana::make_tuple(
        BOOST_HANA_STRING("import"),
        (tok_info) { .kkind = supdef::keyword_kind::import },
        hana::just(BOOST_HOF_LIFT(parse_import))
        
    ),
    hana::make_tuple(
        BOOST_HANA_STRING("embed"),
        (tok_info) { .kkind = supdef::keyword_kind::embed },
        hana::just(BOOST_HOF_LIFT(parse_embed))
    ),
    hana::make_tuple(
        BOOST_HANA_STRING("dump"),
        (tok_info) { .kkind = supdef::keyword_kind::dump },
        hana::just(BOOST_HOF_LIFT(parse_dump))
    ),
    hana::make_tuple(
        BOOST_HANA_STRING("supdef"),
        (tok_info) { .kkind = supdef::keyword_kind::supdef },
        hana::just(BOOST_HOF_LIFT(parse_supdef))
    ),
    hana::make_tuple(
        BOOST_HANA_STRING("runnable"),
        (tok_info) { .kkind = supdef::keyword_kind::runnable },
        hana::just(BOOST_HOF_LIFT(parse_runnable))
    )
);

template <char... Chars>
static constexpr auto get_parser_for_directive(
    hana::string<Chars...>
) {
    using namespace hana::literals;
    auto val = hana::find_if(
        directive_parsers_table, hana::compose(
            hana::equal.to(hana::string<Chars...>{}),
            hana::_[0_c]
        )
    );
    if constexpr (hana::is_just(val))
        return val.value()[2_c];
    else
        return hana::nothing;
}

static_assert(
    hana::is_just(get_parser_for_directive(BOOST_HANA_STRING("import"))),
    "get_parser_for_directive(\"import\") should return a parser"
);
static_assert(
    hana::is_nothing(get_parser_for_directive(BOOST_HANA_STRING("nonexistent"))),
    "get_parser_for_directive(\"nonexistent\") should return nothing"
);


static parse_result<supdef::ast::boolean_node>
parse_boolean(
    points_to_token_and_bidir auto& it,
    const points_to_token_and_bidir auto& begin,
    const points_to_token_and_bidir auto& end,
    bool allow_non_constant = false,
    bool allow_coercion = false
) {
    using namespace supdef;
    using value_type = ast::boolean_node::value_type;
    
    value_type val;
    if (accept_token(it, end, { .tkind = token_kind::boolean_literal }, HORIZWS(1)))
    {
        const token& tok = *next_token(it, end);
        // Case-insensitive comparison, since the lexer should have
        // already handled whether or not we allow things like "True"
        // or "fALsE" etc.
        // If the lexer is case-sensitive, then it didn't match previous
        // examples anyway, and the comparison below is equivalent to
        // the case-sensitive comparison.
        // If the lexer is case-insensitive, then everything's fine
        // since the comparison below is also case-insensitive.
        val = strmatch(tok.data.value(), U"true", false);
    }
    
    if (allow_coercion)
    {
        /* ast::shared_expression expr = parse_expression(it, begin, end);
        if (expr)
        {
            if (allow_non_constant)
        } */
    }
}

static supdef::shared_ptr<supdef::ast::dump_node>
parse_dump(
    points_to_token_and_bidir auto& it,
    const points_to_token_and_bidir auto& begin,
    const points_to_token_and_bidir auto& end
) {
    using namespace supdef;
    throw ast::parse_error(it->loc, "not implemented");
}

static supdef::shared_ptr<supdef::ast::embed_node>
parse_embed(
    points_to_token_and_bidir auto& it,
    const points_to_token_and_bidir auto& begin,
    const points_to_token_and_bidir auto& end
) {
    using namespace supdef;
    throw ast::parse_error(it->loc, "not implemented");
}

static supdef::shared_ptr<supdef::ast::supdef_node>
parse_supdef(
    points_to_token_and_bidir auto& it,
    const points_to_token_and_bidir auto& begin,
    const points_to_token_and_bidir auto& end
) {
    using namespace supdef;
    

}

static supdef::shared_ptr<supdef::ast::import_node>
parse_import(
    points_to_token_and_bidir auto& it,
    const points_to_token_and_bidir auto& begin,
    const points_to_token_and_bidir auto& end
) {
    using namespace supdef;
    typedef std::remove_cvref_t<decltype(it)> iter_type;

    std::u32string imported;

    if (accept_token(it, end, { .tkind = token_kind::string_literal }, HORIZWS(1)) ||
        accept_token(it, end, { .tkind = token_kind::char_literal   }, HORIZWS(1)))
    {
        const token tok = *next_token(it, end);
        imported = tok.data.value_or(U"");
        if (imported.empty())
            throw ast::parse_error(tok.loc, "empty import path");
        expect_token(it, end, { .tkind = token_kind::newline }, HORIZWS(0), "expected newline after import path");
        return make_shared<ast::import_node>(
            tok.loc, stdfs::path(printer::format(std::move(imported)))
        );
    }

    const token path_start = expect_token_alt(
        it, end, { .tkind = token_kind::langle },
        {
            { .tkind = token_kind::string_literal },
            { .tkind = token_kind::char_literal   },
            { .data = U"a file path enclosed in angle brackets (e.g. <path/to/file>)" }
        },
        HORIZWS(1), "check that the format of your path is valid and that "
                    "you leave at least one space to separate it from the "
                    "`import` keyword"
    );
    next_token(it, end);
    size_t path_len = skip_until(
        it, end, token_kind::rangle, [&](const token& tok) -> void {
            imported += tok.data.value_or(U"");
        }
    );
    if (path_len == 0)
        throw ast::parse_error(it->loc, "empty import path");
    const token closingtok = *next_token(it, end);
    assert(closingtok.kind == token_kind::rangle);
    // use import start as the location
    expect_token(it, end, { .tkind = token_kind::newline }, HORIZWS(0), "expected newline after import path");
    return make_shared<ast::import_node>(path_start.loc, stdfs::path(printer::format(std::move(imported))));
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
            if (at_start_of_line(it, begin))
            {
                next_token(it, end);
                // parse a directive

                shared_ptr<ast::node> ret = nullptr;
                hana::for_each(
                    directive_parsers_table,
                    [&](auto&& tup) constexpr {
                        using namespace hana::literals;
                        if (ret)
                            return;
                        auto parser = FWD_AUTO(tup)[2_c];
                        if constexpr (hana::is_just(parser))
                        {
                            if (accept_token(it, end, FWD_AUTO(tup)[1_c], HORIZWS(0)))
                            {
                                next_token(it, end);
                                auto tmp = (parser.value())(it, begin, end);
                                ret = dynamic_pointer_cast<ast::node>(tmp);
                            }
                        }
                    }
                );
                if (ret)
                    return ret;
            }

            // try to parse a builtin function call (e.g. `@len()` etc.)
            // if that fails, then just put the `@` token (and the preceding spaces too)
            // back to the token stream, and parse it as text instead
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