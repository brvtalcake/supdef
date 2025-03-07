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
        )   : node(std::move(loc))
            , block_node(std::move(repl))
            , m_vars{ std::move(indexname), std::move(elemname) }
            , m_iterable(std::move(iterable))
        {
        }

        const std::u32string& indexname() const
        {
            return m_vars.index;
        }
        const std::u32string& elemname() const
        {
            return m_vars.elem;
        }
        const vars& vars() const
        {
            return m_vars;
        }

        const shared_node& iterable() const
        {
            return m_iterable;
        }

        virtual kind node_kind() const override
        {
            return kind::foreachi;
        }

    private:
        vars m_vars;
        shared_node m_iterable;
    };
}