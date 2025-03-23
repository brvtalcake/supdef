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

#include <reFlexLexer.h>

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

std::string supdef::ast::description_string(supdef::ast::node::kind k)
{
    switch (k)
    {
        case supdef::ast::node::kind::import:
            return "import directive";
        case supdef::ast::node::kind::supdef:
            return "supdef directive";
        case supdef::ast::node::kind::runnable:
            return "runnable directive";
        case supdef::ast::node::kind::pragma:
            return "pragma directive";
        case supdef::ast::node::kind::dump:
            return "dump directive";
        case supdef::ast::node::kind::embed:
            return "embed directive";
        case supdef::ast::node::kind::set:
            return "set directive";
        case supdef::ast::node::kind::unset:
            return "unset directive";
        case supdef::ast::node::kind::for_:
            return "for block";
        case supdef::ast::node::kind::foreach:
            return "foreach block";
        case supdef::ast::node::kind::foreachi:
            return "foreachi block";
        case supdef::ast::node::kind::conditional:
            return "conditional block";
        case supdef::ast::node::kind::builtin:
            return "builtin function call expression";
        case supdef::ast::node::kind::varsubst:
            return "variable substitution expression";
        case supdef::ast::node::kind::macrocall:
            return "macro call expression";
        case supdef::ast::node::kind::unaryop:
            return "unary operator expression";
        case supdef::ast::node::kind::binaryop:
            return "binary operator expression";
        case supdef::ast::node::kind::string:
            return "string expression";
        case supdef::ast::node::kind::integer:
            return "integer expression";
        case supdef::ast::node::kind::floating:
            return "floating-point expression";
        case supdef::ast::node::kind::boolean:
            return "boolean expression";
        case supdef::ast::node::kind::list:
            return "list expression";
        case supdef::ast::node::kind::text:
            return "some text";
        default:
            break;
    }
    std::unreachable();
    return "!!! ERROR: unknown node kind (shouldn't have been reached) !!!";
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
    supdef::ast::node::kind parsed_node_kind,
    std::string&& msg,
    const tok_info& expected,
    const supdef::token& got,
    std::optional<supdef::token_loc>&& loc = std::nullopt
) {
    using namespace std::string_literals;
    if (loc)
        return supdef::ast::parse_error(
            *std::move(loc),
            "while trying to parse `"s + supdef::ast::description_string(parsed_node_kind) +
            "`: expected "s + expected.to_string() + ", but got " +
                   tok_info::from(got).to_string() + " instead"   +
                   (msg.empty() ? "" : ": " + std::move(msg))
        );
    else
        return supdef::ast::parse_error(
            got.loc,
            "while trying to parse `"s + supdef::ast::description_string(parsed_node_kind) +
            "`: expected "s + expected.to_string() + ", but got " +
                   tok_info::from(got).to_string() + " instead"   +
                   (msg.empty() ? "" : ": " + std::move(msg))
        );
}

static_assert(std::bool_constant<stdranges::contiguous_range<std::initializer_list<tok_info>>>::value);

static inline supdef::ast::parse_error mk_recursive_parse_error(
    supdef::ast::node::kind parsed_node_kind,
    std::string&& msg,
    std::vector<std::pair<supdef::ast::node::kind, supdef::ast::parse_error>>&& expectations,
    std::optional<supdef::token_loc>&& loc = std::nullopt
) {
    assert(!expectations.empty());

    constexpr auto indent = [](
        const std::string& str,
        size_t start = 0,
        size_t end = std::string::npos
    ) -> std::string {
        static const auto re = std::regex{
            "^(.*)$", std::regex::optimize   |
                      std::regex::ECMAScript |
                      std::regex::multiline
        };
        return std::regex_replace(
            str.substr(start, end), re, "    $1"
        );
    };
    std::string str = "while trying to parse `" + supdef::ast::description_string(parsed_node_kind) +
                   "`: expected one of { " + supdef::ast::description_string(expectations.begin()->first);
    for (const auto& item : expectations | stdviews::drop(1) | ::supdef::drop_last(1))
    {
        str += ", ";
        str += supdef::ast::description_string(item.first);
    }
    str += " or ";
    str += supdef::ast::description_string((expectations.end() - 1)->first);
    str += " }, but got errors instead:";
    for (const auto& [exp, err] : expectations)
    {
        str += "\n";
        str += indent(err.what());
    }
    return supdef::ast::parse_error(
        loc ? *std::move(loc) : std::move(expectations.begin()->second).location(),
        str + (msg.empty() ? "" : "\n--> " + std::move(msg))
    );
}

static inline supdef::ast::parse_error mk_recursive_parse_error(
    supdef::ast::node::kind parsed_node_kind,
    std::string&& msg,
    std::initializer_list<std::pair<supdef::ast::node::kind, supdef::ast::parse_error>> expectations,
    std::optional<supdef::token_loc>&& loc = std::nullopt
) {
    return mk_recursive_parse_error(
        parsed_node_kind,
        std::move(msg),
        std::vector{expectations},
        std::move(loc)
    );
}

static inline supdef::ast::parse_error mk_parse_error(
    supdef::ast::node::kind parsed_node_kind,
    std::string&& msg,
    std::initializer_list<tok_info> expectations,
    const supdef::token& got,
    std::optional<supdef::token_loc>&& loc = std::nullopt
) {
    assert(!expectations.empty());

    using namespace std::string_literals;
    
    if (expectations.size() == 1)
    {
        if (loc)
            return mk_parse_error(std::move(msg), *expectations.begin(), got, *std::move(loc));
        else
            return mk_parse_error(std::move(msg), *expectations.begin(), got, got.loc);
    }
    else if (expectations.size() > 1)
    {
        std::string expected_str = "while trying to parse `"s + supdef::ast::description_string(parsed_node_kind) +
                                "`: expected one of { " + expectations.begin()->to_string();
        for (const auto& it : expectations | stdviews::drop(1) | ::supdef::drop_last(1))
        {
            expected_str += ", ";
            expected_str += it.to_string();
        }
        expected_str += " or ";
        expected_str += (expectations.end() - 1)->to_string();
        expected_str += " }, got " + tok_info::from(got).to_string() + " instead";
        if (loc)
            return supdef::ast::parse_error(
                *std::move(loc),
                expected_str + (msg.empty() ? "" : "\n--> " + std::move(msg))
            );
        else
            return supdef::ast::parse_error(
                got.loc,
                expected_str + (msg.empty() ? "" : "\n--> " + std::move(msg))
            );
    }
#if 1
    std::unreachable();
#else
    else
    {
        return supdef::ast::parse_error(
            loc ? *loc : got.loc,
            "expected any token, got " + tok_info::from(got).to_string() + " instead"
        );
    }
#endif
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
    supdef::ast::node::kind parsed_node_kind,
    const tok_info& to_match,
    bool skip_ws = true,
    size_t min_ws = 1,
    bool skip_newlines = false,
    std::string&& msg = ""
) {
    if (!accept_token(it, end, to_match, skip_ws, min_ws, skip_newlines))
        // TODO: implement backtracking, to be able to continue parsing
        //       after an error has occurred
        throw mk_parse_error(parsed_node_kind, std::move(msg), to_match, *it);
    return *it;
}

static inline ::supdef::token expect_token_alt(
    points_to_token_and_bidir auto& it,
    const points_to_token_and_bidir auto& end,
    supdef::ast::node::kind parsed_node_kind,
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
        throw mk_parse_error(parsed_node_kind, std::move(msg), all_possibilities, *it);
    return *it;
}

#undef  ANYWS
#undef  HORIZWS
#undef  NOWS
#define ANYWS(...) true, (__VA_ARGS__), true
#define HORIZWS(...) true, (__VA_ARGS__), false
#define NOWS false, 0, false

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
static parse_result<supdef::ast::builtin_node>
parse_builtin(
    points_to_token_and_bidir auto& it,
    const points_to_token_and_bidir auto& begin,
    const points_to_token_and_bidir auto& end
);
static parse_result<supdef::ast::varsubst_node>
parse_varsubst(
    points_to_token_and_bidir auto& it,
    const points_to_token_and_bidir auto& begin,
    const points_to_token_and_bidir auto& end
);
static parse_result<supdef::ast::macrocall_node>
parse_macrocall(
    points_to_token_and_bidir auto& it,
    const points_to_token_and_bidir auto& begin,
    const points_to_token_and_bidir auto& end
);
static parse_result<supdef::ast::unaryop_node>
parse_unaryop(
    points_to_token_and_bidir auto& it,
    const points_to_token_and_bidir auto& begin,
    const points_to_token_and_bidir auto& end
);
static parse_result<supdef::ast::binaryop_node>
parse_binaryop(
    points_to_token_and_bidir auto& it,
    const points_to_token_and_bidir auto& begin,
    const points_to_token_and_bidir auto& end
);
static parse_result<supdef::ast::string_node>
parse_string(
    points_to_token_and_bidir auto& it,
    const points_to_token_and_bidir auto& begin,
    const points_to_token_and_bidir auto& end
);
static parse_result<supdef::ast::list_node>
parse_list(
    points_to_token_and_bidir auto& it,
    const points_to_token_and_bidir auto& begin,
    const points_to_token_and_bidir auto& end
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
        hana::type_c<supdef::ast::boolean_node>,
        hana::make_tuple(
            supdef::ast::node::kind::boolean,
            BOOST_HOF_LIFT(parse_boolean)
        )
    ),
    hana::make_pair(
        hana::type_c<supdef::ast::integer_node>,
        hana::make_tuple(
            supdef::ast::node::kind::integer,
            BOOST_HOF_LIFT(parse_integer)
        )
    ),
    hana::make_pair(
        hana::type_c<supdef::ast::floating_node>,
        hana::make_tuple(
            supdef::ast::node::kind::floating,
            BOOST_HOF_LIFT(parse_floating)
        )
    ),
    hana::make_pair(
        hana::type_c<supdef::ast::builtin_node>,
        hana::make_tuple(
            supdef::ast::node::kind::builtin,
            BOOST_HOF_LIFT(parse_builtin)
        )
    ),
    hana::make_pair(
        hana::type_c<supdef::ast::varsubst_node>,
        hana::make_tuple(
            supdef::ast::node::kind::varsubst,
            BOOST_HOF_LIFT(parse_varsubst)
        )
    ),
    hana::make_pair(
        hana::type_c<supdef::ast::macrocall_node>,
        hana::make_tuple(
            supdef::ast::node::kind::macrocall,
            BOOST_HOF_LIFT(parse_macrocall)
        )
    ),
    hana::make_pair(
        hana::type_c<supdef::ast::unaryop_node>,
        hana::make_tuple(
            supdef::ast::node::kind::unaryop,
            BOOST_HOF_LIFT(parse_unaryop)
        )
    ),
    hana::make_pair(
        hana::type_c<supdef::ast::binaryop_node>,
        hana::make_tuple(
            supdef::ast::node::kind::binaryop,
            BOOST_HOF_LIFT(parse_binaryop)
        )
    ),
    hana::make_pair(
        hana::type_c<supdef::ast::string_node>,
        hana::make_tuple(
            supdef::ast::node::kind::string,
            BOOST_HOF_LIFT(parse_string)
        )
    ),
    hana::make_pair(
        hana::type_c<supdef::ast::list_node>,
        hana::make_tuple(
            supdef::ast::node::kind::list,
            BOOST_HOF_LIFT(parse_list)
        )
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

template <
    std::derived_from<supdef::ast::expression_node> NodeT,
    typename TypeListT, typename TypeListIfCoerceT, typename TypeListIfNonConstT
> requires supdef::is_typelist_v<TypeListT>         &&
           supdef::is_typelist_v<TypeListIfCoerceT> &&
           supdef::is_typelist_v<TypeListIfNonConstT>
static parse_result<NodeT>
parse_expr_common(
    std::vector<std::pair<supdef::ast::node::kind, supdef::ast::parse_error>>& errors,
    points_to_token_and_bidir auto& it,
    const points_to_token_and_bidir auto& begin,
    const points_to_token_and_bidir auto& end,
    bool allow_non_constant = false,
    bool allow_coercion = false,
    TypeListT&& type_list = {},
    TypeListIfCoerceT&& type_list_if_coerce = {},
    TypeListIfNonConstT&& type_list_if_non_const = {}
) {
    using namespace supdef;
    constexpr auto mk_ok_ret = [](auto&& maybe_X)
        -> parse_result<NodeT>
            requires std::constructible_from<NodeT, ast::shared_expression>
    {
        return make_shared<NodeT>(
            dynamic_pointer_cast<ast::expression_node>(
                FWD_AUTO(maybe_X).value()
            )
        );
    };
    constexpr auto noop = [](auto&&...) { throw std::logic_error("shouldn't be called"); };
    auto visiting_lambda = [&](auto&& tp) {
        constexpr size_t index = decltype(ic)::value;
        using type_container = decltype(tp);
        using type = typename type_container::type;
        constexpr auto parser = expression_parsers_table[type_container{}].value_or(
            hana::make_tuple(ast::node::kind::unknown, BOOST_HOF_LIFT(noop))
        )[1_c];
        constexpr auto kind = expression_parsers_table[type_container{}].value_or(
            hana::make_tuple(ast::node::kind::unknown, BOOST_HOF_LIFT(noop))
        )[0_c];
        constexpr auto arity_info = supdef::arity_of(parser);
        if (arity_info.max == 5)
        {
            parse_result<type> maybe_X = parser(it, begin, end, allow_non_constant, allow_coercion);
            if (maybe_X)
                return mk_ok_ret(std::move(maybe_X));
            else
                errors.push_back(std::make_pair(kind, std::move(maybe_X).error()));
        }
        else
        {
            parse_result<type> maybe_X = parser(it, begin, end);
            if (maybe_X)
                return mk_ok_ret(std::move(maybe_X));
            else
                errors.push_back(std::make_pair(kind, std::move(maybe_X).error()));
        }
    };
    for (const auto& var : std::move(type_list))
    {
        auto ret = std::visit(visiting_lambda, var);
        if (ret)
            return ret;
    }
    if (allow_coercion)
    {
        for (const auto& var : std::move(type_list_if_coerce))
        {
            auto ret = std::visit(visiting_lambda, var);
            if (ret)
                return ret;
        }
    }
    if (allow_non_constant)
    {
        for (const auto& var : std::move(type_list_if_non_const))
        {
            auto ret = std::visit(visiting_lambda, var);
            if (ret)
                return ret;
        }
    }
    return mk_recursive_parse_error(
        ast::node::kind::expression,
        "please provide a valid expression",
        errors, std::make_optional(it->loc)
    );
}

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
    
    std::vector<std::pair<ast::node::kind, ast::parse_error>> errors{};

    if (accept_token(it, end, { .tkind = token_kind::boolean_literal }, NOWS))
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
        bool matches_true = strmatch(tok.data.value(), U"true", false);
        return make_shared<ast::boolean_node>(ast::boolean_node{tok.loc, matches_true});
    }
    if (accept_token(it, end, { .tkind = token_kind::lparen }, NOWS))
    {
        const token& tok = *next_token(it, end);
        parse_result<ast::boolean_node> maybe_bool = parse_boolean(it, begin, end, allow_non_constant, allow_coercion);
        if (maybe_bool)
            expect_token(
                it, end, ast::node::kind::boolean,
                { .tkind = token_kind::rparen },
                ANYWS(0), "you probably forgot a closing parenthesis"
            );
        return maybe_bool;
    }

    if (allow_non_constant)
    {
        parse_result<ast::varsubst_node> maybe_variable_substitution = parse_varsubst(it, begin, end, false);
        if (maybe_variable_substitution.has_value())
            return make_shared<ast::boolean_node>(
                supdef::dynamic_pointer_cast<ast::expression_node>(
                    std::move(maybe_variable_substitution).value()
                )
            );
        else
            errors.push_back(
                std::make_pair(
                    ast::node::kind::varsubst,
                    std::move(maybe_variable_substitution).error()
                )
            );
        
        parse_result<ast::macrocall_node> maybe_macro_call = parse_macrocall(it, begin, end, false);
        if (maybe_macro_call)
            return make_shared<ast::boolean_node>(
                supdef::dynamic_pointer_cast<ast::expression_node>(
                    std::move(maybe_macro_call).value()
                )
            );
        else
            errors.push_back(
                std::make_pair(
                    ast::node::kind::macrocall,
                    std::move(maybe_macro_call).error()
                )
            );
        
        parse_result<ast::builtin_node> maybe_builtin = parse_builtin(it, begin, end, false);
        if (maybe_builtin)
            return make_shared<ast::boolean_node>(
                supdef::dynamic_pointer_cast<ast::expression_node>(
                    std::move(maybe_builtin).value()
                )
            );
        else
            errors.push_back(
                std::make_pair(
                    ast::node::kind::builtin,
                    std::move(maybe_builtin).error()
                )
            );
    }
    if (allow_coercion)
    {
        parse_result<ast::integer_node> maybe_int = parse_integer(it, begin, end, allow_non_constant, false);
        if (maybe_int)
            return make_shared<ast::boolean_node>(
                supdef::dynamic_pointer_cast<ast::expression_node>(
                    std::move(maybe_int).value()
                )
            );
        else
            errors.push_back(
                std::make_pair(
                    ast::node::kind::integer,
                    std::move(maybe_int).error()
                )
            );

        parse_result<ast::floating_node> maybe_float = parse_floating(it, begin, end, allow_non_constant, false);
        if (maybe_float)
            return make_shared<ast::boolean_node>(
                supdef::dynamic_pointer_cast<ast::expression_node>(
                    std::move(maybe_float).value()
                )
            );
        else
            errors.push_back(
                std::make_pair(
                    ast::node::kind::floating,
                    std::move(maybe_float).error()
                )
            );
    }

    return mk_recursive_parse_error(
        ast::node::kind::boolean, "please provide a valid boolean expression",
        errors, std::make_optional(it->loc)
    );
}

static parse_result<supdef::ast::integer_node>
parse_integer(
    points_to_token_and_bidir auto& it,
    const points_to_token_and_bidir auto& begin,
    const points_to_token_and_bidir auto& end,
    bool allow_non_constant = false,
    bool allow_coercion = false
) {
    using namespace supdef;
    using value_type = ast::integer_node::value_type;
    
    std::vector<std::pair<ast::node::kind, ast::parse_error>> errors{};

    if (accept_token(it, end, { .tkind = token_kind::integer_literal }, NOWS))
    {
        const token& tok = *next_token(it, end);
        return make_shared<ast::integer_node>(tok.loc, tok.data.value());
    }
    if (accept_token(it, end, { .tkind = token_kind::lparen }, NOWS))
    {
        const token& tok = *next_token(it, end);
        parse_result<ast::integer_node> maybe_int = parse_integer(it, begin, end, allow_non_constant, allow_coercion);
        if (maybe_int)
            expect_token(
                it, end, ast::node::kind::integer,
                { .tkind = token_kind::rparen },
                ANYWS(0), "you probably forgot a closing parenthesis"
            );
        return maybe_int;
    }

    if (allow_non_constant)
    {
        parse_result<ast::varsubst_node> maybe_variable_substitution = parse_varsubst(it, begin, end, false);
        if (maybe_variable_substitution.has_value())
            return make_shared<ast::integer_node>(
                supdef::dynamic_pointer_cast<ast::expression_node>(
                    std::move(maybe_variable_substitution).value()
                )
            );
        else
            errors.push_back(
                std::make_pair(
                    ast::node::kind::varsubst,
                    std::move(maybe_variable_substitution).error()
                )
            );
        
        parse_result<ast::macrocall_node> maybe_macro_call = parse_macrocall(it, begin, end, false);
        if (maybe_macro_call)
            return make_shared<ast::integer_node>(
                supdef::dynamic_pointer_cast<ast::expression_node>(
                    std::move(maybe_macro_call).value()
                )
            );
        else
            errors.push_back(
                std::make_pair(
                    ast::node::kind::macrocall,
                    std::move(maybe_macro_call).error()
                )
            );
        
        parse_result<ast::builtin_node> maybe_builtin = parse_builtin(it, begin, end, false);
        if (maybe_builtin)
            return make_shared<ast::integer_node>(
                supdef::dynamic_pointer_cast<ast::expression_node>(
                    std::move(maybe_builtin).value()
                )
            );
        else
            errors.push_back(
                std::make_pair(
                    ast::node::kind::builtin,
                    std::move(maybe_builtin).error()
                )
            );
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
        expect_token(
            it, end, ast::node::kind::import,
            { .tkind = token_kind::newline },
            HORIZWS(0), "there should be a newline after import path"
        );
        return make_shared<ast::import_node>(
            tok.loc, stdfs::path(printer::format(std::move(imported)))
        );
    }

    const token path_start = expect_token_alt(
        it, end, ast::node::kind::import,
        { .tkind = token_kind::langle },
        {
            { .tkind = token_kind::string_literal },
            { .tkind = token_kind::char_literal   },
            { .data = U"a file path enclosed in angle brackets (e.g. <path/to/file>)" }
        },
        HORIZWS(1), "check that the format of your path is valid and that "
                    "you leave at least one space before it, to separate "
                    "it from the `import` keyword"
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
    expect_token(
        it, end, ast::node::kind::import,
        { .tkind = token_kind::newline },
        HORIZWS(0), "there should be a newline after import path"
    );
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