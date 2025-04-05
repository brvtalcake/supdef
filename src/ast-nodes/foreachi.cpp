#include <ast.hpp>

namespace supdef::ast
{
    foreachi_node::foreachi_node(
        ::supdef::token_loc&& loc,
        std::u32string&& indexname,
        std::u32string&& elemname, // a.k.a. varname
        shared_node&& iterable,
        std::vector<shared_node>&& repl
    ) noexcept
        : node(std::move(loc))
        , block_node(std::vector( { std::move(repl) } ))
        , m_vars{ std::move(indexname), std::move(elemname) }
        , m_iterable(std::move(iterable))
    {
    }

    const std::u32string& foreachi_node::indexname() const noexcept
    {
        return m_vars.index;
    }
    const std::u32string& foreachi_node::elemname() const noexcept
    {
        return m_vars.elem;
    }
    const foreachi_node::vars& foreachi_node::variables() const noexcept
    {
        return m_vars;
    }

    const shared_node& foreachi_node::iterable() const noexcept
    {
        return m_iterable;
    }

    const std::vector<shared_node>& foreachi_node::body() const noexcept
    {
        return this->replacement();
    }

    node::kind foreachi_node::node_kind() const noexcept
    {
        return kind::foreachi;
    }
}