namespace supdef::ast
{
    class boolean_node final
        : public virtual node
        , public expression_node
    {
    public:
        boolean_node(::supdef::token_loc&& loc, bool val)
            : node(std::move(loc))
            , expression_node()
            , m_val(val)
        {
        }

        const bool& val() const
        {
            return m_val;
        }

        virtual kind node_kind() const override
        {
            return kind::boolean;
        }

    private:
        bool m_val;
    };
}