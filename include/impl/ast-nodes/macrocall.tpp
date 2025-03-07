namespace supdef::ast
{
    class macrocall_node final
        : public virtual node
        , public expression_node
    {
    public:
        macrocall_node(
            ::supdef::token_loc&& loc,
            std::u32string&& name,
            std::map<std::u32string, shared_node>&& opts,
            std::vector<shared_node>&& args
        )   : node(std::move(loc))
            , expression_node()
            , m_name(std::move(name))
            , m_opts(std::move(opts))
            , m_args(std::move(args))
        {
        }

        const std::u32string& macname() const
        {
            return m_name;
        }

        const std::map<std::u32string, shared_node>& opts() const
        {
            return m_opts;
        }

        const std::vector<shared_node>& args() const
        {
            return m_args;
        }

        virtual kind node_kind() const override
        {
            return kind::macrocall;
        }

    private:
        std::u32string m_name;
        std::map<std::u32string, shared_node> m_opts;
        std::vector<shared_node> m_args;
    };
}