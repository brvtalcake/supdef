#include <ast.hpp>

namespace supdef::ast
{
    foreach_node::foreach_node(
        ::supdef::token_loc&& loc,
        std::u32string&& elemname, // a.k.a. varname
        shared_node&& iterable,
        std::vector<shared_node>&& repl
    ) noexcept
        : node(std::move(loc))
        , block_node(std::vector( { std::move(repl) } ))
        , m_elemname(std::move(elemname))
        , m_iterable(std::move(iterable))
    {
    }

    const std::u32string& foreach_node::elemname() const noexcept
    {
        return m_elemname;
    }
    const std::u32string& foreach_node::varname() const noexcept
    {
        return this->elemname();
    }

    const shared_node& foreach_node::iterable() const noexcept
    {
        return m_iterable;
    }

    const std::vector<shared_node>& foreach_node::body() const noexcept
    {
        return this->replacement();
    }

    virtual node::kind foreach_node::node_kind() const noexcept override
    {
        return kind::foreach;
    }
}