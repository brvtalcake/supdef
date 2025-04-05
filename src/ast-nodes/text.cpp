#include <ast.hpp>

namespace supdef::ast
{
    text_node::text_node(token&& t) noexcept
        : node(t.loc)
        , m_tok(std::move(t))
    {
        assert(m_tok.data != std::nullopt);
    }

    bool text_node::is_identifier() const noexcept
    {
        return m_tok.kind == token_kind::identifier;
    }

    const std::u32string& text_node::text() const noexcept
    {
        return *m_tok.data;
    }

    node::kind text_node::node_kind() const noexcept
    {
        return kind::text;
    }
}