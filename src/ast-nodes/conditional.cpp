#include <ast.hpp>

namespace supdef::ast
{
    conditional_node::conditional_node(
        ::supdef::token_loc&& loc,
        std::vector<shared_node>&& conds,
        std::vector<std::vector<shared_node>>&& repls
    ) noexcept
        : node(std::move(loc))
        , block_node(std::move(repls))
        , m_conds(std::move(conds))
    {
    }

    const std::vector<shared_node>& conditional_node::conds() const noexcept
    {
        return m_conds;
    }

    bool conditional_node::have_else() const noexcept
    {
        return m_conds.size() < this->replacements().size();
    }

    // TODO: maybe the conditions should be shared_expression instead of shared_node
    std::pair<std::optional<shared_node>, std::vector<shared_node>> conditional_node::cond_chain_get(size_t idx) const
    {
        if (idx >= this->replacements().size())
            throw std::out_of_range("idx out of range");
        try
        {
            // for the if's/elseif's
            return {m_conds.at(idx), this->replacement_for(idx)};
        }
        catch (const std::out_of_range&)
        {
            // for the else
            return {std::nullopt, this->replacement_for(idx)};
        }
        std::unreachable();
    }

    virtual node::kind conditional_node::node_kind() const noexcept override
    {
        return kind::conditional;
    }
}