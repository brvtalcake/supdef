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
        )   : node(std::move(loc))
            , block_node(std::move(repl))
            , m_elemname(std::move(elemname))
            , m_iterable(std::move(iterable))
        {
        }

        const std::u32string& elemname() const
        {
            return m_elemname;
        }
        const std::u32string& varname() const
        {
            return this->elemname();
        }

        const shared_node& iterable() const
        {
            return m_iterable;
        }

        virtual kind node_kind() const override
        {
            return kind::foreach;
        }

    private:
        std::u32string m_elemname;
        shared_node m_iterable;
    };
}