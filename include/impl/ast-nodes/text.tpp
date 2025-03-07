namespace supdef::ast
{
    class text_node final
        : public virtual node
    {
    public:
        text_node(token&& t)
            : node(t.loc)
            , m_tok(std::move(t))
        {
        }

        virtual kind node_kind() const override
        {
            return kind::text;
        }

    private:
        token m_tok;
    };
}