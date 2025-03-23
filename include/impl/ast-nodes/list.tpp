namespace supdef::ast
{
    class list_node final
        : public virtual node
        , public expression_node
        , private expression_node_helper_types
    {
        using expression_node_helper_types::helper;

    public:
        using value_type = std::variant<shared_expression, shared_text>;

        list_node(::supdef::token_loc&& loc, std::vector<value_type>&& items)
            : node(std::move(loc))
            , expression_node()
            , m_items(std::move(items))
        {
        }

        const std::vector<value_type>& items() const
        {
            return m_items;
        }

        virtual bool is_constant() const override
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

        virtual bool coerce_to_boolean() const override
        {
            return !m_items.empty();
        }

        virtual supdef::bigint coerce_to_integer() const override
        {
            this->requires_constant();

            throw std::runtime_error("Cannot coerce list to integer");
        }

        virtual supdef::bigfloat coerce_to_floating() const override
        {
            this->requires_constant();

            throw std::runtime_error("Cannot coerce list to floating");
        }

        // TODO: maybe this is not the best way to represent a list as a string
        virtual std::u32string coerce_to_string() const override
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

        virtual kind node_kind() const override
        {
            return kind::list;
        }

    private:
        std::vector<value_type> m_items;
    };
}