namespace supdef::ast
{
    class binaryop_node final
        : public virtual node
        , public expression_node
    {
    public:
        binaryop_node(
            ::supdef::token_loc&& loc,
            std::vector<token>&& op,
            shared_node&& lhs,
            shared_node&& rhs
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

        const std::pair<shared_node, shared_node>& operands() const
        {
            return m_operands;
        }

        virtual kind node_kind() const override
        {
            return kind::binaryop;
        }

    private:
        std::vector<token> m_op;
        std::pair<shared_node, shared_node> m_operands;
    };
}