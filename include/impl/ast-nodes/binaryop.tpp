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
        )   : node(std::move(loc))
            , expression_node()
            , m_op(std::move(op))
            , m_operands(
                std::make_pair(std::move(lhs), std::move(rhs))
            )
        {
        }

        const std::vector<token>& op() const
        {
            return m_op;
        }

        const std::pair<value_type, value_type>& operands() const
        {
            return m_operands;
        }

        virtual bool is_constant() const override
        {
            return m_operands.first ->is_constant() &&
                   m_operands.second->is_constant();
        }

        // can not coerce before evaluating the op, so do not implement

        virtual kind node_kind() const override
        {
            return kind::binaryop;
        }

    private:
        std::vector<token> m_op;
        std::pair<value_type, value_type> m_operands;
    };
}