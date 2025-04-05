#include <ast.hpp>

namespace supdef::ast
{
    list_node::list_node(::supdef::token_loc&& loc, std::vector<value_type>&& items) noexcept
        : node(std::move(loc))
        , expression_node()
        , m_items(std::move(items))
    {
    }

    const std::vector<list_node::value_type>& list_node::items() const noexcept
    {
        return m_items;
    }

    bool list_node::is_constant() const noexcept
    {
        return std::all_of(
            m_items.cbegin(),
            m_items.cend(),
            [](const value_type& item) {
                return std::visit(
                    [](const auto& val) {
                        using helper_type = helper<decltype(val)>;

                        if constexpr (std::same_as<typename helper_type::unqual_val_t, shared_text>)
                            return true;
                        else if constexpr (std::derived_from<typename helper_type::boxed_t, expression_node>)
                            return val->is_constant();
                        return false; // normally unreachable
                    },
                    item
                );
            }
        );
    }

    bool list_node::coerce_to_boolean() const
    {
        return !m_items.empty();
    }

    supdef::bigint list_node::coerce_to_integer() const
    {
        this->requires_constant();

        throw std::runtime_error("Cannot coerce list to integer");
    }

    supdef::bigfloat list_node::coerce_to_floating() const
    {
        this->requires_constant();

        throw std::runtime_error("Cannot coerce list to floating");
    }

    // TODO: maybe this is not the best way to represent a list as a string
    std::u32string list_node::coerce_to_string() const
    {
        this->requires_constant();

        std::u32string result;
        result.push_back(U'[');
        if (!m_items.empty())
        {
            result.append(
                std::visit(
                    [](const auto& val) -> std::u32string {
                        using helper_type = helper<decltype(val)>;

                        if constexpr (std::same_as<typename helper_type::unqual_val_t, shared_text>)
                            return val->text();
                        else if constexpr (std::derived_from<typename helper_type::boxed_t, expression_node>)
                            return val->coerce_to_string();
                        throw std::logic_error("unreachable");
                    },
                    m_items.front()
                )
            );
            for (const auto& item : m_items | stdviews::drop(1))
            {
                result.push_back(U',');
                result.append(
                    std::visit(
                        [](const auto& val) -> std::u32string {
                            using helper_type = helper<decltype(val)>;

                            if constexpr (std::same_as<typename helper_type::unqual_val_t, shared_text>)
                                return val->text();
                            else if constexpr (std::derived_from<typename helper_type::boxed_t, expression_node>)
                                return val->coerce_to_string();
                            throw std::logic_error("unreachable");
                        },
                        item
                    )
                );
            }
        }
        result.push_back(U']');
        return result;
    }

    node::kind list_node::node_kind() const noexcept
    {
        return kind::list;
    }
}