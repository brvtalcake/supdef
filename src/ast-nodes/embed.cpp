#include <ast.hpp>

namespace supdef::ast
{
    // TODO: for now, it will just act as a C preprocessor
    // "#include" directive. Make it accept options later.
    // For instance, the number of bytes to include, the offset
    // from the beginning of the file, whether or not
    // it should be included as a C string literal, etc.
    embed_node::embed_node(::supdef::token_loc&& loc, stdfs::path&& path /*, ... opts */) noexcept
        : node(std::move(loc))
        , directive_node(true)
        , m_path(std::move(path))
    {
    }

    const stdfs::path& embed_node::path() const noexcept
    {
        return m_path;
    }

    node::kind embed_node::node_kind() const noexcept
    {
        return kind::embed;
    }
}