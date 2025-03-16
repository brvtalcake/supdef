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
        )   : node(std::move(loc))
            , expression_node()
            , m_op(std::move(op))
            , m_operand(std::move(operand))
        {
        }

        const std::vector<token>& op() const
        {
            return m_op;
        }

        const value_type& operand() const
        {
            return m_operand;
        }

        virtual bool is_constant() const override
        {
            return m_operand->is_constant();
        }

        // can not coerce before evaluating the op, so do not implement

        virtual kind node_kind() const override
        {
            return kind::unaryop;
        }

    private:
        std::vector<token> m_op;
        value_type m_operand;
    };
}