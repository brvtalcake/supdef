namespace supdef::ast
{
    class list_node final
        : public virtual node
        , public expression_node
    {
    public:
        list_node(::supdef::token_loc&& loc, std::vector<shared_node>&& items)
            : node(std::move(loc))
            , expression_node()
            , m_items(std::move(items))
        {
        }

        const std::vector<shared_node>& items() const
        {
            return m_items;
        }

        virtual kind node_kind() const override
        {
            return kind::list;
        }

    private:
        std::vector<shared_node> m_items;
    };
}