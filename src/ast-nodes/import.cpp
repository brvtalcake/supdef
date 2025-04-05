#include <ast.hpp>

namespace supdef::ast
{
    import_node::import_node(::supdef::token_loc&& loc, stdfs::path&& path) noexcept
        : node(std::move(loc))
        , directive_node(false)
        , m_path(std::move(path))
    {
    }

    import_node::import_node(const ::supdef::token_loc& loc, stdfs::path&& path) noexcept
        : node(loc)
        , directive_node(false)
        , m_path(std::move(path))
    {
    }

    const stdfs::path& import_node::path() const noexcept
    {
        return m_path;
    }

    node::kind import_node::node_kind() const noexcept
    {
        return kind::import;
    }
}