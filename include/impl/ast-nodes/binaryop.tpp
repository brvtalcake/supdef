namespace supdef::ast
{
    class binaryop_node final
        : public virtual node
        , public expression_node
    {
    public:
        using value_type = shared_expression;

        binaryop_node(
            ::supdef::token_loc&& loc,
            std::vector<token>&& op,
            value_type&& lhs,
            value_type&& rhs
        ) noexcept;

        const std::vector<token>& op() const noexcept;

        const std::pair<value_type, value_type>& operands() const noexcept;

        virtual bool is_constant() const noexcept override;

        // can not coerce before evaluating the op, so do not implement

        virtual kind node_kind() const noexcept override;

    private:
        std::vector<token> m_op;
        std::pair<value_type, value_type> m_operands;
    };
}