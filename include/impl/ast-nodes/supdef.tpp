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
        ) noexcept;
        supdef_node(
            ::supdef::token_loc&& loc,
            std::u32string&& name,
            std::map<std::u32string, std::vector<shared_node>>&& opts,
            std::vector<shared_node>&& repl
        ) noexcept;

        const std::vector<shared_node>& replacement() const noexcept;

        const std::map<std::u32string, std::vector<shared_node>>& opts() const noexcept;

        const std::u32string& name() const noexcept;

        virtual kind node_kind() const noexcept override;

    private:
        std::u32string m_name;
        std::map<std::u32string, std::vector<shared_node>> m_opts;
        std::vector<shared_node> m_replacement;
    };
}