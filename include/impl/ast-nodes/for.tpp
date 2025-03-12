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
        )   : node(std::move(loc))
            , block_node(std::vector( { std::move(repl) } ))
            , m_init(std::move(init))
            , m_cond(std::move(cond))
            , m_step(std::move(step))
        {
        }

        const std::pair<std::u32string, shared_node>& init() const
        {
            return m_init;
        }

        const shared_node& cond() const
        {
            return m_cond;
        }

        const std::vector<shared_node>& step() const
        {
            return m_step;
        }

        const std::vector<shared_node>& body() const
        {
            return this->replacement();
        }

        virtual kind node_kind() const override
        {
            return kind::for_;
        }

    private:
        std::pair<std::u32string, shared_node> m_init;
        shared_node m_cond;
        std::vector<shared_node> m_step;
    };
}