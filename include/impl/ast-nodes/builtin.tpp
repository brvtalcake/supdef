namespace supdef::ast
{
    class builtin_node final
        : public virtual node
        , public expression_node
    {
    public:
        builtin_node(
            ::supdef::token_loc&& loc,
            std::u32string&& fname,
            std::vector<shared_node>&& args
        )   : node(std::move(loc))
            , expression_node()
            , m_funcname(std::move(fname))
            , m_args(std::move(args))
        {
        }

        const std::u32string& funcname() const
        {
            return m_funcname;
        }

        const std::vector<shared_node>& args() const
        {
            return m_args;
        }

        virtual kind node_kind() const override
        {
            return kind::builtin;
        }

    private:
        std::u32string m_funcname;
        std::vector<shared_node> m_args;
    };
}