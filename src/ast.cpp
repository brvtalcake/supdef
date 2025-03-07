#include <ast.hpp>
#include <parser.hpp>
#include <tokenizer.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/context.hpp>

namespace x3 = boost::spirit::x3;
namespace qi = boost::spirit::qi;

namespace
{
    struct token_parser : x3::parser<token_parser>
    {
        using attribute_type = ::supdef::token;
        static constexpr bool has_attribute = true;

        constexpr token_parser(::supdef::token_kind kind)
            : parsed_kind(kind)
        {
        }

        constexpr token_parser(::supdef::keyword_kind kind)
            : parsed_kind(::supdef::token_kind::keyword)
            , parsed_keyword(kind)
        {
        }

        constexpr token_parser(::supdef::token_kind kind, std::u32string data)
            : parsed_kind(kind)
            , parsed_data(data)
        {
        }

        ::supdef::token_kind parsed_kind;
        std::optional<::supdef::keyword_kind> parsed_keyword = std::nullopt;
        std::optional<std::u32string> parsed_data = std::nullopt;

        template <typename Iterator, typename Context, typename RContext, typename Attribute>
        bool parse(Iterator& first, Iterator const& last, Context const& context, RContext& rcontext, Attribute& attr) const
        {
            (void)context;
            (void)rcontext;

            bool result = true;

            if (first != last)
            {
                if (first->kind != parsed_kind)
                    result = false;

                if (result && parsed_keyword.has_value() &&
                   (!first->keyword.has_value() || first->keyword.value() != parsed_keyword.value()))
                    result = false;

                if (result && parsed_data.has_value() &&
                   (!first->data.has_value() || first->data.value() != parsed_data.value()))
                    result = false;

                if (result)
                {
                    attr = *first;
                    stdranges::advance(first, 1);
                    return true;
                }
            }

            return false;
        }
    };

    struct any_token_parser : x3::parser<any_token_parser>
    {
        using attribute_type = ::supdef::token;
        static constexpr bool has_attribute = true;

        template <typename Iterator, typename Context, typename RContext, typename Attribute>
        bool parse(Iterator& first, Iterator const& last, Context const& context, RContext& rcontext, Attribute& attr) const
        {
            (void)context;
            (void)rcontext;

            if (first != last)
            {
                attr = *first;
                stdranges::advance(first, 1);
                return true;
            }

            return false;
        }
    };

    struct any_token_but_parser : x3::parser<any_token_but_parser>
    {
        using attribute_type = ::supdef::token;
        static constexpr bool has_attribute = true;

        constexpr any_token_but_parser(::supdef::token_kind kind)
            : parsed_kind(kind)
        {
        }

        constexpr any_token_but_parser(::supdef::keyword_kind kind)
            : parsed_kind(::supdef::token_kind::keyword)
            , parsed_keyword(kind)
        {
        }

        constexpr any_token_but_parser(::supdef::token_kind kind, std::u32string data)
            : parsed_kind(kind)
            , parsed_data(data)
        {
        }

        ::supdef::token_kind parsed_kind;
        std::optional<::supdef::keyword_kind> parsed_keyword = std::nullopt;
        std::optional<std::u32string> parsed_data = std::nullopt;

        template <typename Iterator, typename Context, typename RContext, typename Attribute>
        bool parse(Iterator& first, Iterator const& last, Context const& context, RContext& rcontext, Attribute& attr) const
        {
            (void)context;
            (void)rcontext;

            if (first != last)
            {
                if (first->kind == parsed_kind)
                {
                    if (parsed_keyword.has_value() &&
                       (first->keyword.has_value() && first->keyword.value() == parsed_keyword.value()))
                        return false;

                    if (parsed_data.has_value() &&
                       (first->data.has_value() && first->data.value() == parsed_data.value()))
                        return false;

                    attr = *first;
                    stdranges::advance(first, 1);
                    return true;
                }
            }

            return false;
        }
    };

    static const auto langid_parser = (
        any_token_but_parser{::supdef::token_kind::horizontal_whitespace}
    );

    static const auto pragma_runnable_lang_parser = (
        token_parser{::supdef::keyword_kind::runnable} >> +token_parser{::supdef::token_kind::horizontal_whitespace} >>
        langid_parser >> +token_parser{::supdef::token_kind::horizontal_whitespace} >>
        token_parser{::supdef::token_kind::identifier} >> +token_parser{::supdef::token_kind::horizontal_whitespace} >>
        +(any_token_parser{} - (token_parser{::supdef::token_kind::newline} | token_parser{::supdef::token_kind::eof}))
    );

    static const auto pragma_runnable_parser = (
        token_parser{::supdef::keyword_kind::runnable} >> +token_parser{::supdef::token_kind::horizontal_whitespace} >>
        token_parser{::supdef::token_kind::identifier} >> +token_parser{::supdef::token_kind::horizontal_whitespace} >>
        +(any_token_parser{} - (token_parser{::supdef::token_kind::newline} | token_parser{::supdef::token_kind::eof}))
    );

    static const auto pragma_supdef_parser = (
        token_parser{::supdef::keyword_kind::supdef} >> +token_parser{::supdef::token_kind::horizontal_whitespace} >>
        token_parser{::supdef::token_kind::identifier} >> +token_parser{::supdef::token_kind::horizontal_whitespace} >>
        +(any_token_parser{} - (token_parser{::supdef::token_kind::newline} | token_parser{::supdef::token_kind::eof}))
    );

    static const auto pragma_other_parser = (
        token_parser{::supdef::token_kind::identifier} >> +token_parser{::supdef::token_kind::horizontal_whitespace} >>
        +(any_token_parser{} - (token_parser{::supdef::token_kind::newline} | token_parser{::supdef::token_kind::eof}))
    );

    static const auto pragma_parser = (
        *token_parser{::supdef::token_kind::horizontal_whitespace} >> token_parser{::supdef::token_kind::at}       >>
        *token_parser{::supdef::token_kind::horizontal_whitespace} >> token_parser{::supdef::keyword_kind::pragma} >>
        +token_parser{::supdef::token_kind::horizontal_whitespace} >> (
            pragma_runnable_lang_parser | pragma_runnable_parser | pragma_supdef_parser | pragma_other_parser
        )
    );
}