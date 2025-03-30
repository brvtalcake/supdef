namespace supdef::ast
{
    class foreach_node final
        : public virtual node
        , public block_node
    {
    public:
        foreach_node(
            ::supdef::token_loc&& loc,
            std::u32string&& elemname, // a.k.a. varname
            shared_node&& iterable,
            std::vector<shared_node>&& repl
        ) noexcept;

        const std::u32string& elemname() const noexcept;
        const std::u32string& varname() const noexcept;

        const shared_node& iterable() const noexcept;

        const std::vector<shared_node>& body() const noexcept;

        virtual kind node_kind() const noexcept override;

    private:
        std::u32string m_elemname;
        shared_node m_iterable;
    };
}