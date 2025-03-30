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

        list_node(::supdef::token_loc&& loc, std::vector<value_type>&& items) noexcept;

        const std::vector<value_type>& items() const noexcept;

        virtual bool is_constant() const noexcept override;

        virtual bool coerce_to_boolean() const override;

        virtual supdef::bigint coerce_to_integer() const override;

        virtual supdef::bigfloat coerce_to_floating() const override;

        // TODO: maybe this is not the best way to represent a list as a string
        virtual std::u32string coerce_to_string() const override;

        virtual kind node_kind() const noexcept override;

    private:
        std::vector<value_type> m_items;
    };
}