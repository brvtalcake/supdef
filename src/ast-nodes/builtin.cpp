#include <ast.hpp>

namespace supdef::ast
{
    builtin_node::builtin_node(
        ::supdef::token_loc&& loc,
        std::u32string&& fname,
        std::vector<builtin_node::value_type>&& args
    ) noexcept
        : node(std::move(loc))
        , expression_node()
        , m_funcname(std::move(fname))
        , m_args(std::move(args))
    {
    }

    const std::u32string& builtin_node::funcname() const noexcept
    {
        return m_funcname;
    }

    const std::vector<builtin_node::value_type>& builtin_node::args() const noexcept
    {
        return m_args;
    }

    bool builtin_node::is_constant() const noexcept
    {
        return std::all_of(
            m_args.cbegin(),
            m_args.cend(),
            [](const value_type& arg) {
                return std::visit(
                    [](const auto& val) {
                        using helper_type = helper<decltype(val)>;
    
                        if constexpr (std::same_as<typename helper_type::unqual_val_t, shared_text>)
                            return true;
                        else if constexpr (std::derived_from<typename helper_type::boxed_t, expression_node>)
                            return val->is_constant();
                        return false; // normally unreachable
                    },
                    arg
                );
            }
        );
    }

    node::kind builtin_node::node_kind() const noexcept
    {
        return kind::builtin;
    }
}