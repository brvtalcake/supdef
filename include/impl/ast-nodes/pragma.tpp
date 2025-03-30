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
            float_env,
            integer_env,
            other
        };
        pragma_node(
            ::supdef::token_loc&& loc,
            pragma_kind k,
            std::optional<interpreter::lang_identifier>&& lang,
            std::u32string&& path,
            std::vector<shared_node>&& value
        ) noexcept;

        pragma_kind prag_kind() const noexcept;

        const std::optional<interpreter::lang_identifier>& lang() const noexcept;

        const std::u32string& pragma() const noexcept;

        const std::vector<shared_node>& value() const noexcept;

        virtual kind node_kind() const noexcept override;

    private:
        pragma_kind m_kind;
        std::optional<interpreter::lang_identifier> m_lang;
        std::u32string m_prag;
        std::vector<shared_node> m_val;
    };
}