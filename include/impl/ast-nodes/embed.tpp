namespace supdef::ast
{
    class embed_node final
        : public virtual node
        , public directive_node
    {
    public:
        // TODO: for now, it will just act as a C preprocessor
        // "#include" directive. Make it accept options later.
        // For instance, the number of bytes to include, the offset
        // from the beginning of the file, whether or not
        // it should be included as a C string literal, etc.
        embed_node(::supdef::token_loc&& loc, stdfs::path&& path /*, ... opts */) noexcept;

        const stdfs::path& path() const noexcept;

        virtual kind node_kind() const noexcept override;

    private:
        stdfs::path m_path;
    };
}