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
        ) noexcept;

        const std::u32string& varname() const noexcept;

        bool global() const noexcept;

        virtual kind node_kind() const noexcept override;

    private:
        std::u32string m_varname;
        bool m_global;
    };
}