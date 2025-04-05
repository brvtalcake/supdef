#include <ast.hpp>

namespace supdef::ast
{
    binaryop_node::binaryop_node(
        ::supdef::token_loc&& loc,
        std::vector<token>&& op,
        binaryop_node::value_type&& lhs,
        binaryop_node::value_type&& rhs
    ) noexcept
        : node(std::move(loc))
        , expression_node()
        , m_op(std::move(op))
        , m_operands(
            std::make_pair(std::move(lhs), std::move(rhs))
        )
    {
    }

    const std::vector<token>& binaryop_node::op() const noexcept
    {
        return m_op;
    }

    const std::pair<binaryop_node::value_type, binaryop_node::value_type>&
    binaryop_node::operands() const noexcept
    {
        return m_operands;
    }

    bool binaryop_node::is_constant() const noexcept
    {
        return m_operands.first ->is_constant() &&
               m_operands.second->is_constant();
    }

    node::kind binaryop_node::node_kind() const noexcept
    {
        return kind::binaryop;
    }
}