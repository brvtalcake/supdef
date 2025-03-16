namespace supdef::ast
{
    class directive_node
        : public virtual node
    {
    protected:
        directive_node(bool outputs)
            : m_outputs(outputs)
        {
        }

    public:
        directive_node(::supdef::token_loc&& loc, bool outputs)
            : node(std::move(loc))
            , m_outputs(outputs)
        {
        }

        bool outputs() const
        {
            return m_outputs;
        }

        virtual kind node_kind() const override
        {
            return kind::directive;
        }

    private:
        bool m_outputs;
    };

    class expression_node
        : public virtual node
    {
    protected:
        expression_node()
        {
            supdef::bigfloat::set_precision(256);
        }

        // This function is called by the coercion functions to ensure that the
        // coercion is only done on constant expressions.
        void requires_constant() const
        {
            if (!this->is_constant())
                throw std::runtime_error("non-constant coercion needs interpreter to first evaluate expression");
        }

    public:
        expression_node(::supdef::token_loc&& loc)
            : node(std::move(loc))
        {
        }

        virtual kind node_kind() const override
        {
            return kind::expression;
        }

        virtual bool is_constant() const = 0;
        
        virtual bool coerce_to_boolean() const = 0;
        virtual supdef::bigint coerce_to_integer() const = 0;
        virtual supdef::bigfloat coerce_to_floating() const = 0;
        virtual std::u32string coerce_to_string() const = 0;
    };

    class block_node
        : public virtual node
    {
    protected:
        block_node(std::vector<std::vector<shared_node>>&& repl)
            : m_replacements(std::move(repl))
        {
        }

    public:
        block_node(::supdef::token_loc&& loc, std::vector<std::vector<shared_node>>&& repl)
            : node(std::move(loc))
            , m_replacements(std::move(repl))
        {
        }

        const std::vector<std::vector<shared_node>>& replacements() const
        {
            return m_replacements;
        }
        const std::vector<shared_node>& replacement_for(size_t index) const
        {
            return m_replacements.at(index);
        }
        const std::vector<shared_node>& replacement() const
        {
            return m_replacements.front();
        }

        virtual kind node_kind() const override
        {
            return kind::block;
        }

    private:
        std::vector<std::vector<shared_node>> m_replacements;
    };

    struct expression_node_helper_types
    {
        template <typename ValT>
        struct helper
        {
            using val_t = ValT;
            using unqual_val_t = std::remove_cvref_t<val_t>;
                
            constexpr bool is_box = supdef::has_boxed_type<val_t>;
            struct unused_local_type {};
            using boxed_t = std::conditional_t<is_box, std::remove_cvref_t<supdef::boxed_type_t<val_t>>, unused_local_type>;
        };
    };
}