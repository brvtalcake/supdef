#include <ast.hpp>

namespace supdef::ast
{
    macrocall_node::macrocall_node(
        ::supdef::token_loc&& loc,
        macrocall_node::macro_value_type&& name,
        std::map<std::u32string, shared_node>&& opts,
        std::vector<macrocall_node::args_value_type>&& args
    ) noexcept
        : node(std::move(loc))
        , expression_node()
        , m_name(std::move(name))
        , m_opts(std::move(opts))
        , m_args(std::move(args))
    {
    }

    const macrocall_node::macro_value_type&
    macrocall_node::macname() const noexcept
    {
        return m_name;
    }

    const std::map<std::u32string, shared_node>&
    macrocall_node::opts() const noexcept
    {
        return m_opts;
    }

    const std::vector<macrocall_node::args_value_type>&
    macrocall_node::args() const noexcept
    {
        return m_args;
    }

    virtual bool macrocall_node::is_constant() const noexcept override
    {
        bool values[] = {
            std::all_of(
                m_args.cbegin(),
                m_args.cend(),
                [](const args_value_type& arg) {
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
            ),
            std::visit(
                [](const auto& val) {
                    using helper_type = helper<decltype(val)>;

                    if constexpr (std::same_as<typename helper_type::unqual_val_t, shared_text>)
                        return true;
                    else if constexpr (std::derived_from<typename helper_type::boxed_t, expression_node>)
                        return val->is_constant();
                    return false; // normally unreachable
                },
                m_name
            )
        };
        return stdranges::all_of(values, std::identity{});
    }

    // can not coerce before evaluating the call, so do not implement

    virtual node::kind macrocall_node::node_kind() const noexcept override
    {
        return kind::macrocall;
    }
}