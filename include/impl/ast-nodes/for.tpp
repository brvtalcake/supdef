namespace supdef::ast
{
    class for_node final
        : public virtual node
        , public block_node
    {
    public:
        for_node(
            ::supdef::token_loc&& loc,
            std::pair<std::u32string, shared_node>&& init,
            shared_node&& cond,
            std::vector<shared_node>&& step,
            std::vector<shared_node>&& repl
        ) noexcept;

        const std::pair<std::u32string, shared_node>& init() const noexcept;

        const shared_node& cond() const noexcept;

        const std::vector<shared_node>& step() const noexcept;

        const std::vector<shared_node>& body() const noexcept;

        virtual kind node_kind() const noexcept override;

    private:
        std::pair<std::u32string, shared_node> m_init;
        shared_node m_cond;
        std::vector<shared_node> m_step;
    };
}