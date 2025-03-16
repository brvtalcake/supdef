namespace supdef::ast
{
    class string_node final
        : public virtual node
        , public expression_node
        , private expression_node_helper_types
    {
        using expression_node_helper_types::helper;

    public:
        enum class quote_kind
        { singly, doubly };
    
    private:
        using first_alternative = std::pair<std::u32string, quote_kind>;
        using second_alternative = shared_expression;
    public:

        using value_type = std::variant<first_alternative, second_alternative>;

        string_node(token&& t)
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

        string_node(::supdef::token_loc&& loc, value_type&& str)
            : node(std::move(loc))
            , expression_node()
            , m_str(std::move(str))
        {
        }

        const value_type& val() const
        {
            return m_str;
        }

        bool is_literal() const
        {
            return std::holds_alternative<first_alternative>(m_str);
        }

        bool is_computed() const
        {
            return std::holds_alternative<shared_expression>(m_str);
        }

        std::optional<quote_kind> quoted() const
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

        virtual bool is_constant() const override
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

        virtual std::u32string coerce_to_string() const override
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

        virtual kind node_kind() const override
        {
            return kind::string;
        }

    private:
        value_type m_str;
        quote_kind m_qkind;
    };
}