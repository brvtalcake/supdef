#include <ast.hpp>

namespace supdef::ast
{
    runnable_node::runnable_node(
        ::supdef::token_loc&& loc,
        std::u32string&& name,
        interpreter::lang_identifier&& lang,
        std::map<std::u32string, std::vector<shared_node>>&& opts,
        std::vector<token>&& code
    ) noexcept
        : node(std::move(loc))
        , directive_node(false)
        , m_name(std::move(name))
        , m_lang(std::move(lang))
        , m_opts(std::move(opts))
        , m_code(std::move(code))
    {
    }

    const std::u32string& runnable_node::name() const noexcept
    {
        return m_name;
    }

    const interpreter::lang_identifier&
    runnable_node::lang() const noexcept
    {
        return m_lang;
    }

    const std::map<std::u32string, std::vector<shared_node>>&
    runnable_node::opts() const noexcept
    {
        return m_opts;
    }

    const std::vector<token>& runnable_node::code() const noexcept
    {
        return m_code;
    }

    virtual node::kind runnable_node::node_kind() const noexcept override
    {
        return kind::runnable;
    }
}