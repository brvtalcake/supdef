namespace supdef::ast
{
    class string_node final
        : public virtual node
        , public expression_node
        , private expression_node_helper_types
    {
        using expression_node_helper_types::helper;

    public:
        enum class quote_kind
        { singly, doubly };
    
    private:
        using first_alternative = std::pair<std::u32string, quote_kind>;
        using second_alternative = shared_expression;

    public:

        using value_type = std::variant<first_alternative, second_alternative>;

        string_node(token&& t) noexcept;

        string_node(::supdef::token_loc&& loc, value_type&& str) noexcept;

        const value_type& val() const noexcept;

        bool is_literal() const noexcept;

        bool is_computed() const noexcept;

        std::optional<quote_kind> quoted() const noexcept;

        virtual bool is_constant() const noexcept override;

        // for coercion to bool / int / float, the interpreter must first reparse the string contents as a
        // bool / int / float, so do not implement

        virtual std::u32string coerce_to_string() const override;

        virtual kind node_kind() const noexcept override;

    private:
        value_type m_str;
        quote_kind m_qkind;
    };
}