namespace supdef::ast
{
    class import_node final
        : public virtual node
        , public directive_node
    {
    public:
        import_node(::supdef::token_loc&& loc, stdfs::path&& path)
            : node(std::move(loc))
            , directive_node(false)
            , m_path(std::move(path))
        {
        }

        const stdfs::path& path() const
        {
            return m_path;
        }

        virtual kind node_kind() const override
        {
            return kind::import;
        }

    private:
        stdfs::path m_path;
    };
}
