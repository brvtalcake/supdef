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
        )   : node(std::move(loc))
            , directive_node(false)
            , m_name(std::move(name))
            , m_lang(std::move(lang))
            , m_opts(std::move(opts))
            , m_code(std::move(code))
        {
        }

        const std::u32string& name() const
        {
            return m_name;
        }

        const interpreter::lang_identifier& lang() const
        {
            return m_lang;
        }

        const std::map<std::u32string, std::vector<shared_node>>& opts() const
        {
            return m_opts;
        }

        const std::vector<token>& code() const
        {
            return m_code;
        }

        virtual kind node_kind() const override
        {
            return kind::runnable;
        }
    private:
        std::u32string m_name;
        interpreter::lang_identifier m_lang;
        std::map<std::u32string, std::vector<shared_node>> m_opts;
        std::vector<token> m_code;
    };
}