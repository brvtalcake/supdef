namespace supdef::ast
{
    class varsubst_node final
        : public virtual node
        , public expression_node
    {
    public:
        varsubst_node(
            ::supdef::token_loc&& loc,
            std::variant<std::u32string, size_t>&& varname
        )   : node(std::move(loc))
            , expression_node()
            , m_varname(std::move(varname))
        {
        }
    
        const std::variant<std::u32string, size_t>& varname() const
        {
            return m_varname;
        }

        bool arg() const
        {
            return std::holds_alternative<size_t>(m_varname);
        }
        
        bool var() const
        {
            return std::holds_alternative<std::u32string>(m_varname);
        }

        virtual kind node_kind() const override
        {
            return kind::varsubst;
        }
    
    private:
        std::variant<std::u32string, size_t> m_varname;
    };
}