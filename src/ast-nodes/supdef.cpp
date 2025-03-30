#include <ast.hpp>

namespace supdef::ast
{
    supdef_node::supdef_node(
        ::supdef::token_loc&& loc,
        std::u32string&& name,
        std::vector<shared_node>&& repl
    ) noexcept
        : node(std::move(loc))
        , directive_node(false) // call to a supdef outputs, but not its definition
        , m_name(std::move(name))
        , m_opts()
        , m_replacement(std::move(repl))
    {
    }
    supdef_node::supdef_node(
        ::supdef::token_loc&& loc,
        std::u32string&& name,
        std::map<std::u32string, std::vector<shared_node>>&& opts,
        std::vector<shared_node>&& repl
    ) noexcept
        : node(std::move(loc))
        , directive_node(false)
        , m_name(std::move(name))
        , m_opts(std::move(opts))
        , m_replacement(std::move(repl))
    {
    }

    const std::vector<shared_node>& supdef_node::replacement() const noexcept
    {
        return m_replacement;
    }

    const std::map<std::u32string, std::vector<shared_node>>& supdef_node::opts() const noexcept
    {
        return m_opts;
    }

    const std::u32string& supdef_node::name() const noexcept
    {
        return m_name;
    }

    virtual node::kind supdef_node::node_kind() const noexcept override
    {
        return kind::supdef;
    }
}