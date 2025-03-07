namespace supdef::ast
{
    class unset_node final
        : public virtual node
        , public directive_node
    {
    public:
        unset_node(
            ::supdef::token_loc&& loc,
            std::u32string&& varname,
            bool global = false
        )   : node(std::move(loc))
            , directive_node(false)
            , m_varname(std::move(varname))
            , m_global(global)
        {
        }

        const std::u32string& varname() const
        {
            return m_varname;
        }

        bool global() const
        {
            return m_global;
        }

        virtual kind node_kind() const override
        {
            return kind::unset;
        }

    private:
        std::u32string m_varname;
        bool m_global;
    };
}