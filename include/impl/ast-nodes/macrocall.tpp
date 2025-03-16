namespace supdef::ast
{
    class macrocall_node final
        : public virtual node
        , public expression_node
        , private expression_node_helper_types
    {
        using expression_node_helper_types::helper;

    public:
        using macro_value_type = std::variant<shared_text, shared_expression>;
        using args_value_type = std::variant<shared_text, shared_expression>;

        macrocall_node(
            ::supdef::token_loc&& loc,
            macro_value_type&& name,
            std::map<std::u32string, shared_node>&& opts,
            std::vector<args_value_type>&& args
        )   : node(std::move(loc))
            , expression_node()
            , m_name(std::move(name))
            , m_opts(std::move(opts))
            , m_args(std::move(args))
        {
        }

        const macro_value_type& macname() const
        {
            return m_name;
        }

        const std::map<std::u32string, shared_node>& opts() const
        {
            return m_opts;
        }

        const std::vector<args_value_type>& args() const
        {
            return m_args;
        }

        virtual bool is_constant() const override
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

        virtual kind node_kind() const override
        {
            return kind::macrocall;
        }

    private:
        macro_value_type m_name;
        std::map<std::u32string, shared_node> m_opts;
        std::vector<args_value_type> m_args;
    };
}