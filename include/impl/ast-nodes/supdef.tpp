namespace supdef::ast
{
    class supdef_node final
        : public virtual node
        , public directive_node
    {
    public:
        supdef_node(
            ::supdef::token_loc&& loc,
            std::u32string&& name,
            std::vector<shared_node>&& repl
        )   : node(std::move(loc))
            , directive_node(false) // call to a supdef outputs, but not its definition
            , m_name(std::move(name))
            , m_opts()
            , m_replacement(std::move(repl))
        {
        }
        supdef_node(
            ::supdef::token_loc&& loc,
            std::u32string&& name,
            std::map<std::u32string, std::vector<shared_node>>&& opts,
            std::vector<shared_node>&& repl
        )   : node(std::move(loc))
            , directive_node(false)
            , m_name(std::move(name))
            , m_opts(std::move(opts))
            , m_replacement(std::move(repl))
        {
        }

        const std::vector<shared_node>& replacement() const
        {
            return m_replacement;
        }

        const std::map<std::u32string, std::vector<shared_node>>& opts() const
        {
            return m_opts;
        }

        const std::u32string& name() const
        {
            return m_name;
        }

        virtual kind node_kind() const override
        {
            return kind::supdef;
        }

    private:
        std::u32string m_name;
        std::map<std::u32string, std::vector<shared_node>> m_opts;
        std::vector<shared_node> m_replacement;
    };
}