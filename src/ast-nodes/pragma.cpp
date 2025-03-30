#include <ast.hpp>

namespace supdef::ast
{
    pragma_node::pragma_node(
        ::supdef::token_loc&& loc,
        pragma_node::pragma_kind k,
        std::optional<interpreter::lang_identifier>&& lang,
        std::u32string&& path,
        std::vector<shared_node>&& value
    ) noexcept
        : node(std::move(loc))
        , directive_node(false)
        , m_kind(k)
        , m_lang(std::move(lang))
        , m_prag(std::move(path))
        , m_val(std::move(value))
    {
    }

    pragma_node::pragma_kind pragma_node::prag_kind() const noexcept
    {
        return m_kind;
    }

    const std::optional<interpreter::lang_identifier>& pragma_node::lang() const noexcept
    {
        return m_lang;
    }

    const std::u32string& pragma_node::pragma() const noexcept
    {
        return m_prag;
    }

    const std::vector<shared_node>& pragma_node::value() const noexcept
    {
        return m_val;
    }

    virtual node::kind pragma_node::node_kind() const noexcept override
    {
        return kind::pragma;
    }
}