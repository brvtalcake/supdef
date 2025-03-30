#include <ast.hpp>


namespace supdef::ast
{
    void floating_node::set_value_from_string(const std::u32string& val)
    {
        if (val.empty())
        {
            m_val = ::supdef::bigfloat();
            return;
        }

        std::string str;

        for (char32_t c : val)
        {
            const auto nval = ::supdef::unicode::numeric_value<long long>(c);

            if (!nval.has_value())
                str.push_back((char)c);
            else if ((long long)'0' + *nval < (long long)std::numeric_limits<char>::min() ||
                    (long long)'0' + *nval > (long long)std::numeric_limits<char>::max())
                throw std::runtime_error("Invalid character in floating literal");
            else
                str.push_back((char)((long long)'0' + *nval));
        }

        m_val = ::supdef::bigfloat(std::move(str));
    }

    floating_node::floating_node(::supdef::token_loc&& loc, ::supdef::ast::shared_expression&& val) noexcept
        : node(std::move(loc))
        , expression_node()
        , m_val(std::move(val))
    {
    }

    floating_node::floating_node(const ::supdef::token_loc& loc, ::supdef::ast::shared_expression&& val) noexcept
        : node(loc)
        , expression_node()
        , m_val(std::move(val))
    {
    }

    floating_node::floating_node(::supdef::token_loc&& loc, const std::u32string& val)
        : node(std::move(loc))
        , expression_node()
        , m_val()
    {
        this->set_value_from_string(val);
    }

    floating_node::floating_node(const ::supdef::token_loc& loc, const std::u32string& val)
        : node(loc)
        , expression_node()
        , m_val()
    {
        this->set_value_from_string(val);
    }

    floating_node::floating_node(const ::supdef::token& tok)
        : node(tok.loc)
        , expression_node()
        , m_val()
    {
        this->set_value_from_string(tok.value());
    }

    const floating_node::value_type& floating_node::val() const noexcept
    {
        return m_val;
    }

    virtual bool floating_node::is_constant() const noexcept override
    {
        return std::visit(
            [](const auto& val) {
                using helper_type = helper<decltype(val)>;

                if constexpr (std::same_as<typename helper_type::unqual_val_t, ::supdef::bigfloat>)
                    return true;
                else if constexpr (std::derived_from<typename helper_type::boxed_t, expression_node>)
                    return val->is_constant();
                return false; // normally unreachable
            },
            m_val
        );
    }

    virtual bool floating_node::coerce_to_boolean() const override
    {
        this->requires_constant();

        return std::visit(
            [](const auto& val) -> bool {
                using helper_type = helper<decltype(val)>;

                if constexpr (std::same_as<typename helper_type::unqual_val_t, ::supdef::bigfloat>)
                    return val.template as<bool>();
                else if constexpr (std::derived_from<typename helper_type::boxed_t, expression_node>)
                    return val->coerce_to_boolean();
                throw std::logic_error("unreachable");
            },
            m_val
        );
    }

    virtual supdef::bigint floating_node::coerce_to_integer() const override
    {
        this->requires_constant();

        return std::visit(
            [](const auto& val) -> supdef::bigint {
                using helper_type = helper<decltype(val)>;

                if constexpr (std::same_as<typename helper_type::unqual_val_t, ::supdef::bigfloat>)
                    return val.template as<supdef::bigint>();
                else if constexpr (std::derived_from<typename helper_type::boxed_t, expression_node>)
                    return val->coerce_to_integer();
                throw std::logic_error("unreachable");
            },
            m_val
        );
    }
    
    virtual supdef::bigfloat floating_node::coerce_to_floating() const override
    {
        this->requires_constant();

        return std::visit(
            [](const auto& val) -> supdef::bigfloat {
                using helper_type = helper<decltype(val)>;

                if constexpr (std::same_as<typename helper_type::unqual_val_t, ::supdef::bigfloat>)
                    return val;
                else if constexpr (std::derived_from<typename helper_type::boxed_t, expression_node>)
                    return val->coerce_to_floating();
                throw std::logic_error("unreachable");
            },
            m_val
        );
    }

    virtual std::u32string floating_node::coerce_to_string() const override
    {
        this->requires_constant();

        return std::visit(
            [](const auto& val) -> std::u32string {
                using helper_type = helper<decltype(val)>;

                if constexpr (std::same_as<typename helper_type::unqual_val_t, ::supdef::bigfloat>)
                    return supdef::printer::unformat(val.str());
                else if constexpr (std::derived_from<typename helper_type::boxed_t, expression_node>)
                    return val->coerce_to_string();
                throw std::logic_error("unreachable");
            },
            m_val
        );
    }

    virtual node::kind floating_node::node_kind() const noexcept override
    {
        return kind::floating;
    }
}