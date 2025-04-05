#include <ast.hpp>

namespace supdef::ast
{
    set_node::set_node(
        ::supdef::token_loc&& loc,
        std::u32string&& varname,
        std::vector<shared_node>&& value,
        bool global,
        bool defer
    ) noexcept
        : node(std::move(loc))
        , directive_node(false)
        , m_varname(std::move(varname))
        , m_value(std::move(value))
        , m_global(global)
        , m_defer(defer)
    {
    }

    const std::u32string& set_node::varname() const noexcept
    {
        return m_varname;
    }

    const std::vector<shared_node>& set_node::value() const noexcept
    {
        return m_value;
    }

    bool set_node::global() const noexcept
    {
        return m_global;
    }

    bool set_node::defered_expansion() const noexcept
    {
        return m_defer;
    }

    node::kind set_node::node_kind() const noexcept
    {
        return kind::set;
    }
}