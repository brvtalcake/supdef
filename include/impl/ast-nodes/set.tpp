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
        ) noexcept;

        const std::u32string& varname() const noexcept;

        const std::vector<shared_node>& value() const noexcept;

        bool global() const noexcept;

        bool defered_expansion() const noexcept;

        virtual kind node_kind() const noexcept override;

    private:
        std::u32string m_varname;
        std::vector<shared_node> m_value;
        bool m_global,
             m_defer;
    };
}