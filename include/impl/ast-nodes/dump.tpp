namespace supdef::ast
{
    class dump_node final
        : public virtual node
        , public directive_node
    {
    public:
        dump_node(::supdef::token_loc&& loc, size_t i)
            : node(std::move(loc))
            , directive_node(true)
            , m_index(i)
        {
        }

        size_t index() const
        {
            return m_index;
        }

        virtual kind node_kind() const override
        {
            return kind::dump;
        }

    private:
        size_t m_index;
    };
}