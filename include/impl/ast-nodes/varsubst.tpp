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
        ) noexcept;
    
        const value_type& varname() const noexcept;

        bool arg() const noexcept;
        
        bool var() const noexcept;

        bool invalid() const noexcept;

        virtual bool is_constant() const noexcept override;

        // can not coerce before evaluating the substitution, so do not implement

        virtual kind node_kind() const noexcept override;
    
    private:
        value_type m_varname;
    };
}