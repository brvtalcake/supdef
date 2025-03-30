#include <ast.hpp>

namespace supdef::ast
{
    string_node::string_node(token&& t) noexcept
        : node(std::move(t.loc))
        , expression_node()
        , m_str(
            std::make_pair(
                *std::move(t.data),
                t.kind == token_kind::char_literal ?
                    quote_kind::singly             :
                    quote_kind::doubly
            )
        )
    {
    }

    string_node::string_node(::supdef::token_loc&& loc, string_node::value_type&& str) noexcept
        : node(std::move(loc))
        , expression_node()
        , m_str(std::move(str))
    {
    }

    const string_node::value_type& string_node::val() const noexcept
    {
        return m_str;
    }

    bool string_node::is_literal() const noexcept
    {
        return std::holds_alternative<first_alternative>(m_str);
    }

    bool string_node::is_computed() const noexcept
    {
        return std::holds_alternative<shared_expression>(m_str);
    }

    std::optional<string_node::quote_kind> string_node::quoted() const noexcept
    {
        return std::visit(
            [](const auto& val) -> std::optional<quote_kind> {
                using helper_type = helper<decltype(val)>;

                if constexpr (std::same_as<typename helper_type::unqual_val_t, first_alternative>)
                    return val.second;
                return std::nullopt;
            },
            m_str
        );
    }

    virtual bool string_node::is_constant() const noexcept override
    {
        return std::visit(
            [](const auto& val) {
                using helper_type = helper<decltype(val)>;

                if constexpr (std::same_as<typename helper_type::unqual_val_t, first_alternative>)
                    return true;
                else if constexpr (std::derived_from<typename helper_type::boxed_t, expression_node>)
                    return val->is_constant();
                return false; // normally unreachable
            },
            m_str
        );
    }

    // for coercion to bool / int / float, the interpreter must first reparse the string contents as a
    // bool / int / float, so do not implement

    virtual std::u32string string_node::coerce_to_string() const override
    {
        this->requires_constant();

        return std::visit(
            [](const auto& val) -> std::u32string {
                using helper_type = helper<decltype(val)>;

                if constexpr (std::same_as<typename helper_type::unqual_val_t, first_alternative>)
                    return val.first;
                else if constexpr (std::derived_from<typename helper_type::boxed_t, expression_node>)
                    return val->coerce_to_string();
                throw std::logic_error("unreachable");
            },
            m_str
        );
    }

    virtual node::kind string_node::node_kind() const noexcept override
    {
        return kind::string;
    }
}