namespace supdef::ast
{
    class foreachi_node final
        : public virtual node
        , public block_node
    {
    public:
        struct vars
        {
            std::u32string index;
            std::u32string elem;
        };

        foreachi_node(
            ::supdef::token_loc&& loc,
            std::u32string&& indexname,
            std::u32string&& elemname, // a.k.a. varname
            shared_node&& iterable,
            std::vector<shared_node>&& repl
        ) noexcept;

        const std::u32string& indexname() const noexcept;
        const std::u32string& elemname() const noexcept;
        const vars& variables() const noexcept;

        const shared_node& iterable() const noexcept;

        const std::vector<shared_node>& body() const noexcept;

        virtual kind node_kind() const noexcept override;

    private:
        vars m_vars;
        shared_node m_iterable;
    };
}