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

        boolean_node(::supdef::token_loc&& loc, value_type&& val) noexcept;

        boolean_node(const ::supdef::token_loc& loc, value_type&& val) noexcept;

        boolean_node(shared_expression&& val) noexcept;

        const value_type& val() const noexcept;

        virtual bool is_constant() const noexcept override;

        virtual bool coerce_to_boolean() const override;

        virtual supdef::bigint coerce_to_integer() const override;

        virtual supdef::bigfloat coerce_to_floating() const override;

        virtual std::u32string coerce_to_string() const override;

        virtual kind node_kind() const noexcept override;

    private:
        value_type m_val;
    };
}