namespace supdef::ast
{
    class integer_node final
        : public virtual node
        , public expression_node
        , private expression_node_helper_types
    {
        using expression_node_helper_types::helper;

    public:
        using value_type = std::variant<::supdef::bigint, shared_expression>;

        integer_node(::supdef::token_loc&& loc, value_type&& val)
            : node(std::move(loc))
            , expression_node()
            , m_val(std::move(val))
        {
        }
        integer_node(::supdef::token_loc&& loc, const std::u32string& val)
            : node(std::move(loc))
            , expression_node()
            , m_val()
        {
            std::string str;
            for (char32_t c : val)
            {
                // TODO: maybe already checked while tokenizing ?
                const auto nval = ::supdef::unicode::numeric_value<long long>(c);
                if (!nval.has_value() ||
                    (long long)'0' + *nval < (long long)std::numeric_limits<char>::min() ||
                    (long long)'0' + *nval > (long long)std::numeric_limits<char>::max())
                    throw std::runtime_error("Invalid character in integer literal");
                str.push_back((char)((long long)'0' + *nval));
            }
            m_val = ::supdef::bigint(std::move(str));
        }
        integer_node(const ::supdef::token_loc& loc, value_type&& val)
            : node(loc)
            , expression_node()
            , m_val(std::move(val))
        {
        }
        integer_node(const ::supdef::token_loc& loc, const std::u32string& val)
            : node(loc)
            , expression_node()
            , m_val()
        {
            std::string str;
            for (char32_t c : val)
            {
                // TODO: maybe already checked while tokenizing ?
                const auto nval = ::supdef::unicode::numeric_value<long long>(c);
                if (!nval.has_value() ||
                    (long long)'0' + *nval < (long long)std::numeric_limits<char>::min() ||
                    (long long)'0' + *nval > (long long)std::numeric_limits<char>::max())
                    throw std::runtime_error("Invalid character in integer literal");
                str.push_back((char)((long long)'0' + *nval));
            }
            m_val = ::supdef::bigint(std::move(str));
        }

        const value_type& val() const
        {
            return m_val;
        }

        virtual bool is_constant() const override
        {
            return std::visit(
                [](const auto& val) {
                    using helper_type = helper<decltype(val)>;

                    if constexpr (std::same_as<typename helper_type::unqual_val_t, ::supdef::bigint>)
                        return true;
                    else if constexpr (std::derived_from<typename helper_type::boxed_t, expression_node>)
                        return val->is_constant();
                    return false; // normally unreachable
                },
                m_val
            );
        }

        virtual bool coerce_to_boolean() const override
        {
            this->requires_constant();

            return std::visit(
                [](const auto& val) -> bool {
                    using helper_type = helper<decltype(val)>;

                    if constexpr (std::same_as<typename helper_type::unqual_val_t, ::supdef::bigint>)
                        return val.template as<bool>();
                    else if constexpr (std::derived_from<typename helper_type::boxed_t, expression_node>)
                        return val->coerce_to_boolean();
                    throw std::logic_error("unreachable");
                },
                m_val
            );
        }

        virtual ::supdef::bigint coerce_to_integer() const override
        {
            this->requires_constant();

            return std::visit(
                [](const auto& val) -> ::supdef::bigint {
                    using helper_type = helper<decltype(val)>;

                    if constexpr (std::same_as<typename helper_type::unqual_val_t, ::supdef::bigint>)
                        return val;
                    else if constexpr (std::derived_from<typename helper_type::boxed_t, expression_node>)
                        return val->coerce_to_integer();
                    throw std::logic_error("unreachable");
                },
                m_val
            );
        }

        virtual ::supdef::bigfloat coerce_to_floating() const override
        {
            this->requires_constant();

            return std::visit(
                [](const auto& val) -> ::supdef::bigfloat {
                    using helper_type = helper<decltype(val)>;

                    if constexpr (std::same_as<typename helper_type::unqual_val_t, ::supdef::bigint>)
                        return val.template as<::supdef::bigfloat>();
                    else if constexpr (std::derived_from<typename helper_type::boxed_t, expression_node>)
                        return val->coerce_to_floating();
                    throw std::logic_error("unreachable");
                },
                m_val
            );
        }

        virtual std::u32string coerce_to_string() const override
        {
            this->requires_constant();

            return std::visit(
                [](const auto& val) -> std::u32string {
                    using helper_type = helper<decltype(val)>;

                    if constexpr (std::same_as<typename helper_type::unqual_val_t, ::supdef::bigint>)
                        return supdef::printer::unformat(val.str());
                    else if constexpr (std::derived_from<typename helper_type::boxed_t, expression_node>)
                        return val->coerce_to_string();
                    throw std::logic_error("unreachable");
                },
                m_val
            );
        }

        virtual kind node_kind() const override
        {
            return kind::integer;
        }

    private:
        value_type m_val;
    };
}