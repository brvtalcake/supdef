namespace supdef::ast
{
    class set_node final
        : public virtual node
        , public directive_node
    {
    public:
        set_node(
            ::supdef::token_loc&& loc,
            std::u32string&& varname,
            std::vector<shared_node>&& value,
            bool global = false,
            bool defer = false
        )   : node(std::move(loc))
            , directive_node(false)
            , m_varname(std::move(varname))
            , m_value(std::move(value))
            , m_global(global)
            , m_defer(defer)
        {
        }

        const std::u32string& varname() const
        {
            return m_varname;
        }

        const std::vector<shared_node>& value() const
        {
            return m_value;
        }

        bool global() const
        {
            return m_global;
        }

        bool defered_expansion() const
        {
            return m_defer;
        }

        virtual kind node_kind() const override
        {
            return kind::set;
        }

    private:
        std::u32string m_varname;
        std::vector<shared_node> m_value;
        bool m_global,
             m_defer;
    };
}