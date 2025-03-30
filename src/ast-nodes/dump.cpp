#include <ast.hpp>

namespace supdef::ast
{
    dump_node::dump_node(::supdef::token_loc&& loc, size_t i) noexcept
        : node(std::move(loc))
        , directive_node(true)
        , m_index(i)
    {
    }

    size_t dump_node::index() const noexcept
    {
        return m_index;
    }

    virtual node::kind dump_node::node_kind() const noexcept override
    {
        return kind::dump;
    }
}