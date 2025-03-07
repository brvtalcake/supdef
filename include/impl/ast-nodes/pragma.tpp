namespace supdef::ast
{
    class pragma_node final
        : public virtual node
        , public directive_node
    {
    public:
        enum class pragma_kind
        {
            supdef,
            runnable,
            runnable_lang,
            other
        };
        pragma_node(
            ::supdef::token_loc&& loc,
            pragma_kind k,
            std::optional<interpreter::lang_identifier>&& lang,
            std::u32string&& path,
            std::vector<shared_node>&& value
        )   : node(std::move(loc))
            , directive_node(false)
            , m_kind(k)
            , m_lang(std::move(lang))
            , m_prag(std::move(path))
            , m_val(std::move(value))
        {
        }

        pragma_kind prag_kind() const
        {
            return m_kind;
        }

        const std::optional<interpreter::lang_identifier>& lang() const
        {
            return m_lang;
        }

        const std::u32string& pragma() const
        {
            return m_prag;
        }

        const std::vector<shared_node>& value() const
        {
            return m_val;
        }

        virtual kind node_kind() const override
        {
            return kind::pragma;
        }

    private:
        pragma_kind m_kind;
        std::optional<interpreter::lang_identifier> m_lang;
        std::u32string m_prag;
        std::vector<shared_node> m_val;
    };
}