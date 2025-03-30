namespace supdef::ast
{
    class dump_node final
        : public virtual node
        , public directive_node
    {
    public:
        dump_node(::supdef::token_loc&& loc, size_t i) noexcept;

        size_t index() const noexcept;

        virtual kind node_kind() const noexcept override;

    private:
        size_t m_index;
    };
}