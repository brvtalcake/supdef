#include <ast.hpp>

namespace supdef::ast
{
    for_node::for_node(
        ::supdef::token_loc&& loc,
        std::pair<std::u32string, shared_node>&& init,
        shared_node&& cond,
        std::vector<shared_node>&& step,
        std::vector<shared_node>&& repl
    ) noexcept
        : node(std::move(loc))
        , block_node(std::vector( { std::move(repl) } ))
        , m_init(std::move(init))
        , m_cond(std::move(cond))
        , m_step(std::move(step))
    {
    }

    const std::pair<std::u32string, shared_node>& for_node::init() const noexcept
    {
        return m_init;
    }

    const shared_node& for_node::cond() const noexcept
    {
        return m_cond;
    }

    const std::vector<shared_node>& for_node::step() const noexcept
    {
        return m_step;
    }

    const std::vector<shared_node>& for_node::body() const noexcept
    {
        return this->replacement();
    }

    virtual node::kind for_node::node_kind() const noexcept override
    {
        return kind::for_;
    }
}