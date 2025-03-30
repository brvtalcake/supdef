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
        ) noexcept;

        const macro_value_type& macname() const noexcept;

        const std::map<std::u32string, shared_node>& opts() const noexcept;

        const std::vector<args_value_type>& args() const noexcept;

        virtual bool is_constant() const noexcept override;

        // can not coerce before evaluating the call, so do not implement

        virtual kind node_kind() const noexcept override;

    private:
        macro_value_type m_name;
        std::map<std::u32string, shared_node> m_opts;
        std::vector<args_value_type> m_args;
    };
}