namespace supdef::ast
{
    class text_node final
        : public virtual node
    {
    public:
        text_node(token&& t) noexcept;

        bool is_identifier() const noexcept;

        const std::u32string& text() const noexcept;

        virtual kind node_kind() const noexcept override;

    private:
        token m_tok;
    };
}