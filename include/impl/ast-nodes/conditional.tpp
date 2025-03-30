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
        ) noexcept;

        const std::vector<shared_node>& conds() const noexcept;

        bool have_else() const noexcept;

        // TODO: maybe the conditions should be shared_expression instead of shared_node
        std::pair<std::optional<shared_node>, std::vector<shared_node>> cond_chain_get(size_t idx) const;

        virtual kind node_kind() const noexcept override;

    private:
        std::vector<shared_node> m_conds;
    };
}