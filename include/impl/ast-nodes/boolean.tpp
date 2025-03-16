namespace supdef::ast
{
    class boolean_node final
        : public virtual node
        , public expression_node
        , private expression_node_helper_types
    {
        using expression_node_helper_types::helper;

    public:
        using value_type = std::variant<bool, shared_expression>;

        boolean_node(::supdef::token_loc&& loc, value_type&& val)
            : node(std::move(loc))
            , expression_node()
            , m_val(std::move(val))
        {
        }

        const value_type& val() const
        {
            return m_val;
        }

        virtual bool is_constant() const override
        {
            return std::visit(
                [](const auto& val) {
                    using helper_type = helper<decltype(val)>;

                    if constexpr (std::same_as<typename helper_type::unqual_val_t, bool>)
                        return true;
                    else if constexpr (std::derived_from<typename helper_type::boxed_t, expression_node>)
                        return val->is_constant();
                    return false; // normally unreachable
                },
                m_val
            );
        }

        virtual bool coerce_to_boolean() const override
        {
            this->requires_constant();

            return std::visit(
                [](const auto& val) -> bool {
                    using helper_type = helper<decltype(val)>;

                    if constexpr (std::same_as<typename helper_type::unqual_val_t, bool>)
                        return val;
                    else if constexpr (std::derived_from<typename helper_type::boxed_t, expression_node>)
                        return val->coerce_to_boolean();
                    throw std::logic_error("unreachable");
                },
                m_val
            );
        }

        virtual supdef::bigint coerce_to_integer() const override
        {
            this->requires_constant();

            return std::visit(
                [](const auto& val) -> supdef::bigint {
                    using helper_type = helper<decltype(val)>;

                    if constexpr (std::same_as<typename helper_type::unqual_val_t, bool>)
                        return val ? (unsigned long)1 : (unsigned long)0;
                    else if constexpr (std::derived_from<typename helper_type::boxed_t, expression_node>)
                        return val->coerce_to_integer();
                    throw std::logic_error("unreachable");
                },
                m_val
            );
        }

        virtual supdef::bigfloat coerce_to_floating() const override
        {
            this->requires_constant();

            return std::visit(
                [](const auto& val) -> supdef::bigfloat {
                    using namespace std::string_view_literals;

                    using helper_type = helper<decltype(val)>;

                    if constexpr (std::same_as<typename helper_type::unqual_val_t, bool>)
                        return supdef::bigfloat(val ? "1"sv : "0"sv);
                    else if constexpr (std::derived_from<typename helper_type::boxed_t, expression_node>)
                        return val->coerce_to_floating();
                    throw std::logic_error("unreachable");
                },
                m_val
            );
        }

        virtual std::u32string coerce_to_string() const override
        {
            this->requires_constant();

            return std::visit(
                [](const auto& val) -> std::u32string {
                    using helper_type = helper<decltype(val)>;

                    if constexpr (std::same_as<typename helper_type::unqual_val_t, bool>)
                        return val ? U"true" : U"false";
                    else if constexpr (std::derived_from<typename helper_type::boxed_t, expression_node>)
                        return val->coerce_to_string();
                    throw std::logic_error("unreachable");
                },
                m_val
            );
        }

        virtual kind node_kind() const override
        {
            return kind::boolean;
        }

    private:
        value_type m_val;
    };
}