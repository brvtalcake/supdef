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
            assert(m_tok.data != std::nullopt);
        }

        bool is_identifier() const
        {
            return m_tok.kind == token_kind::identifier;
        }

        const std::u32string& text() const
        {
            return *m_tok.data;
        }

        virtual kind node_kind() const override
        {
            return kind::text;
        }

    private:
        token m_tok;
    };
}