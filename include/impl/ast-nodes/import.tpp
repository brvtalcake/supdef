namespace supdef::ast
{
    class import_node final
        : public virtual node
        , public directive_node
    {
    public:
        import_node(::supdef::token_loc&& loc, stdfs::path&& path) noexcept;

        import_node(const ::supdef::token_loc& loc, stdfs::path&& path) noexcept;

        const stdfs::path& path() const noexcept;

        virtual kind node_kind() const noexcept override;

    private:
        stdfs::path m_path;
    };
}
