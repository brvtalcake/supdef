#ifdef __INTELLISENSE__
#include <types.hpp>
#endif

namespace supdef::ast
{
    class directive_node
        : public virtual node
    {
    protected:
        directive_node(bool outputs) noexcept;

    public:
        directive_node(::supdef::token_loc&& loc, bool outputs) noexcept;

        bool outputs() const noexcept;

        virtual std::ostream& do_output_to(std::ostream& os) const override;

        virtual kind node_kind() const noexcept override;

    private:
        bool m_outputs;
    };

    class expression_node
        : public virtual node
    {
    protected:
        expression_node() noexcept = default;

        // This function is called by the coercion functions to ensure that the
        // coercion is only done on constant expressions.
        void requires_constant() const;

    public:
        expression_node(::supdef::token_loc&& loc) noexcept;

        virtual std::ostream& do_output_to(std::ostream& os) const override;

        virtual kind node_kind() const noexcept override;

        virtual bool is_constant() const noexcept = 0;
        
        virtual bool coerce_to_boolean() const = 0;
        virtual supdef::bigint coerce_to_integer() const = 0;
        virtual supdef::bigfloat coerce_to_floating() const = 0;
        virtual std::u32string coerce_to_string() const = 0;
    };

    class block_node
        : public virtual node
    {
    protected:
        block_node(std::vector<std::vector<shared_node>>&& repl) noexcept;

    public:
        block_node(::supdef::token_loc&& loc, std::vector<std::vector<shared_node>>&& repl) noexcept;

        const std::vector<std::vector<shared_node>>& replacements() const noexcept;
        const std::vector<shared_node>& replacement_for(size_t index) const;
        const std::vector<shared_node>& replacement() const;

        virtual std::ostream& do_output_to(std::ostream& os) const override;

        virtual kind node_kind() const noexcept override;

    private:
        std::vector<std::vector<shared_node>> m_replacements;
    };

    struct call_expr_node_helper_types
    {
        // when user calls a macro or a builtin like...
        //    macro($var1, , 42)
        // ... then the second argument is a void argument
        // which is represented by this struct
        struct void_arg_placeholder
        { };

        static constexpr void_arg_placeholder void_arg{};

        // when user calls a macro or a builtin like...
        //    macro($var1, 42 43.5689       "txt", 42)
        // ... then the second argument is a delimited argument
        // which is represented by this struct
        template <typename... Variants>
        struct delimited_arg_placeholder
            : public std::vector<std::variant<Variants...>>
        { };
    };

    struct expression_node_helper_types
    {
        template <typename ValT>
        struct helper
        {
            using val_t = ValT;
            using unqual_val_t = std::remove_cvref_t<val_t>;
                
            static constexpr bool is_box = supdef::has_boxed_type<val_t>;
            struct unused_local_type {};
            using boxed_t = typename decltype(
                hana::if_(
                    hana::bool_c<is_box>,
                    [](auto t) {
                        return hana::type_c<
                            std::remove_cvref_t<
                                supdef::boxed_type_t<
                                    typename decltype(t)::type
                                >
                            >
                        >;
                    },
                    [](auto) {
                        return hana::type_c<unused_local_type>;
                    }
                )(hana::type_c<val_t>)
            )::type;
        };
    };
}

static_assert(
    std::same_as<
        supdef::ast::expression_node_helper_types::helper<int>::boxed_t,
        supdef::ast::expression_node_helper_types::helper<int>::unused_local_type
    >, "supdef::ast::expression_node_helper_types::helper is broken"
);
static_assert(
    std::same_as<
        supdef::ast::expression_node_helper_types::helper<int[]>::boxed_t,
        int
    >, "supdef::ast::expression_node_helper_types::helper is broken"
);
static_assert(
    std::same_as<
        supdef::ast::expression_node_helper_types::helper<int[5]>::boxed_t,
        int
    >, "supdef::ast::expression_node_helper_types::helper is broken"
);
static_assert(
    std::same_as<
        supdef::ast::expression_node_helper_types::helper<int*>::boxed_t,
        int
    >, "supdef::ast::expression_node_helper_types::helper is broken"
);
static_assert(
    std::same_as<
        supdef::ast::expression_node_helper_types::helper<int**>::boxed_t,
        int*
    >, "supdef::ast::expression_node_helper_types::helper is broken"
);
static_assert(
    std::same_as<
        typename supdef::ast::expression_node_helper_types::helper<supdef::shared_ptr<int>>::boxed_t,
        int
    >, "supdef::ast::expression_node_helper_types::helper is broken"
);
static_assert(
    std::same_as<
        typename supdef::ast::expression_node_helper_types::helper<supdef::shared_ptr<int[]>>::boxed_t,
        int
    >, "supdef::ast::expression_node_helper_types::helper is broken"
);