namespace supdef::ast
{
    class conditional_node final
        : public virtual node
        , public block_node
    {
    public:
        conditional_node(
            ::supdef::token_loc&& loc,
            std::vector<shared_node>&& conds,
            std::vector<std::vector<shared_node>>&& repls
        )   : node(std::move(loc))
            , block_node(std::move(repls))
            , m_conds(std::move(conds))
        {
        }
        const std::vector<shared_node>& conds() const
        {
            return m_conds;
        }

        bool have_else() const
        {
            return m_conds.size() < this->replacements().size();
        }

        std::pair<std::optional<shared_node>, std::vector<shared_node>> cond_chain_get(size_t idx) const
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

        virtual kind node_kind() const override
        {
            return kind::conditional;
        }

    private:
        std::vector<shared_node> m_conds;
    };
}