namespace supdef::ast
{
    class unaryop_node final
        : public virtual node
        , public expression_node
    {
    public:
        unaryop_node(
            ::supdef::token_loc&& loc,
            std::vector<token>&& op,
            shared_node&& operand
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

        const shared_node& operand() const
        {
            return m_operand;
        }

        virtual kind node_kind() const override
        {
            return kind::unaryop;
        }

    private:
        std::vector<token> m_op;
        shared_node m_operand;
    };
}