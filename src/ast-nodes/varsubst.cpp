#include <ast.hpp>

namespace supdef::ast
{
    varsubst_node::varsubst_node(
        ::supdef::token_loc&& loc,
        varsubst_node::value_type&& varname
    ) noexcept
        : node(std::move(loc))
        , expression_node()
        , m_varname(std::move(varname))
    {
    }

    const varsubst_node::value_type& varsubst_node::varname() const noexcept
    {
        return m_varname;
    }

    bool varsubst_node::arg() const noexcept
    {
        return std::holds_alternative<shared_expression>(m_varname) &&
               std::get<shared_expression>(m_varname)->is(kind::integer);
    }
    
    bool varsubst_node::var() const noexcept
    {
        return std::holds_alternative<shared_text>(m_varname) &&
               std::get<shared_text>(m_varname)->is_identifier();
    }

    bool varsubst_node::invalid() const noexcept
    {
        return !arg() && !var();
    }

    bool varsubst_node::is_constant() const noexcept
    {
        // must be interpreted to substitute, first
        // after that only the result can be constant
        return false;
    }

    // can not coerce before evaluating the substitution, so do not implement

    node::kind varsubst_node::node_kind() const noexcept
    {
        return kind::varsubst;
    }
}