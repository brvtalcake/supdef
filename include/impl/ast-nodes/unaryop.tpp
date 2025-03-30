namespace supdef::ast
{
    class unaryop_node final
        : public virtual node
        , public expression_node
        , private expression_node_helper_types
    {
        using expression_node_helper_types::helper;

    public:
        using value_type = shared_expression;

        unaryop_node(
            ::supdef::token_loc&& loc,
            std::vector<token>&& op,
            value_type&& operand
        ) noexcept;

        const std::vector<token>& op() const noexcept;

        const value_type& operand() const noexcept;

        virtual bool is_constant() const noexcept override;

        // can not coerce before evaluating the op, so do not implement

        virtual kind node_kind() const noexcept override;

    private:
        std::vector<token> m_op;
        value_type m_operand;
    };
}