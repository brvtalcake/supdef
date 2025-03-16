namespace supdef::ast
{
    class builtin_node final
        : public virtual node
        , public expression_node
        , private expression_node_helper_types
    {
        using expression_node_helper_types::helper;

    public:
        using value_type = std::variant<shared_text, shared_expression>;

        builtin_node(
            ::supdef::token_loc&& loc,
            std::u32string&& fname,
            std::vector<value_type>&& args
        )   : node(std::move(loc))
            , expression_node()
            , m_funcname(std::move(fname))
            , m_args(std::move(args))
        {
        }

        const std::u32string& funcname() const
        {
            return m_funcname;
        }

        const std::vector<value_type>& args() const
        {
            return m_args;
        }

        virtual bool is_constant() const override
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
        
        // can not coerce before evaluating the call, so do not implement

        virtual kind node_kind() const override
        {
            return kind::builtin;
        }

    private:
        std::u32string m_funcname;
        std::vector<value_type> m_args;
    };
}