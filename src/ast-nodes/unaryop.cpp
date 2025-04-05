#include <ast.hpp>

namespace supdef::ast
{
    unaryop_node::unaryop_node(
        ::supdef::token_loc&& loc,
        std::vector<token>&& op,
        unaryop_node::value_type&& operand
    ) noexcept
        : node(std::move(loc))
        , expression_node()
        , m_op(std::move(op))
        , m_operand(std::move(operand))
    {
    }

    const std::vector<token>& unaryop_node::op() const noexcept
    {
        return m_op;
    }

    const value_type& unaryop_node::operand() const noexcept
    {
        return m_operand;
    }

    bool unaryop_node::is_constant() const noexcept
    {
        return m_operand->is_constant();
    }

    // can not coerce before evaluating the op, so do not implement

    node::kind unaryop_node::node_kind() const noexcept
    {
        return kind::unaryop;
    }
}