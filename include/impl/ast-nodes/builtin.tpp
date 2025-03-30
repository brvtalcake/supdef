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
        ) noexcept;

        const std::u32string& funcname() const noexcept;

        const std::vector<value_type>& args() const noexcept;

        virtual bool is_constant() const noexcept override;
        
        // can not coerce before evaluating the call, so do not implement

        virtual kind node_kind() const noexcept override;

    private:
        std::u32string m_funcname;
        std::vector<value_type> m_args;
    };
}