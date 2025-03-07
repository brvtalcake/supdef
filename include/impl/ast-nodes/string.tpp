namespace supdef::ast
{
    class string_node final
        : public virtual node
        , public expression_node
    {
    public:
        enum class quote_kind
        { singly, doubly };

        string_node(token&& t)
            : node(std::move(t.loc))
            , expression_node()
            , m_str(*std::move(t.data))
            , m_qkind(
                t.kind == token_kind::char_literal ?
                    quote_kind::singly             :
                    quote_kind::doubly)
        {
        }

        const std::u32string& str() const
        {
            return m_str;
        }

        quote_kind quoted() const
        {
            return m_qkind;
        }

        virtual kind node_kind() const override
        {
            return kind::string;
        }

    private:
        std::u32string m_str;
        quote_kind m_qkind;
    };
}