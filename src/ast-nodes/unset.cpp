#include <ast.hpp>

namespace supdef::ast
{
    unset_node::unset_node(
        ::supdef::token_loc&& loc,
        std::u32string&& varname,
        bool global
    ) noexcept
        : node(std::move(loc))
        , directive_node(false)
        , m_varname(std::move(varname))
        , m_global(global)
    {
    }

    const std::u32string& unset_node::varname() const noexcept
    {
        return m_varname;
    }

    bool unset_node::global() const noexcept
    {
        return m_global;
    }

    virtual node::kind unset_node::node_kind() const noexcept override
    {
        return kind::unset;
    }
}