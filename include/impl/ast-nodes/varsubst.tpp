namespace supdef::ast
{
    class varsubst_node final
        : public virtual node
        , public expression_node
        , private expression_node_helper_types
    {
        using expression_node_helper_types::helper;

    public:
        using value_type = std::variant<shared_text, shared_expression>;

        varsubst_node(
            ::supdef::token_loc&& loc,
            value_type&& varname
        )   : node(std::move(loc))
            , expression_node()
            , m_varname(std::move(varname))
        {
        }
    
        const value_type& varname() const
        {
            return m_varname;
        }

        bool arg() const
        {
            return std::holds_alternative<shared_expression>(m_varname) &&
                   std::get<shared_expression>(m_varname)->is(kind::integer);
        }
        
        bool var() const
        {
            return std::holds_alternative<shared_text>(m_varname) &&
                   std::get<shared_text>(m_varname)->is_identifier();
        }

        bool invalid() const
        {
            return !arg() && !var();
        }

        virtual bool is_constant() const override
        {
            // must be interpreted to substitute, first
            // after that only the result can be constant
            return false;
        }

        // can not coerce before evaluating the substitution, so do not implement

        virtual kind node_kind() const override
        {
            return kind::varsubst;
        }
    
    private:
        value_type m_varname;
    };
}