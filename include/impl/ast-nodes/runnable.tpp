namespace supdef::ast
{
    class runnable_node final
        : public virtual node
        , public directive_node
    {
    public:
        runnable_node(
            ::supdef::token_loc&& loc,
            std::u32string&& name,
            interpreter::lang_identifier&& lang,
            std::map<std::u32string, std::vector<shared_node>>&& opts,
            std::vector<token>&& code
        ) noexcept;

        const std::u32string& name() const noexcept;

        const interpreter::lang_identifier& lang() const noexcept;

        const std::map<std::u32string, std::vector<shared_node>>& opts() const noexcept;

        const std::vector<token>& code() const noexcept;

        virtual kind node_kind() const noexcept override;

    private:
        std::u32string m_name;
        interpreter::lang_identifier m_lang;
        std::map<std::u32string, std::vector<shared_node>> m_opts;
        std::vector<token> m_code;
    };
}