namespace supdef::ast
{
    class floating_node final
        : public virtual node
        , public expression_node
    {
    public:
        floating_node(::supdef::token_loc&& loc, ::supdef::bigfloat&& val)
            : node(std::move(loc))
            , expression_node()
            , m_val(std::move(val))
        {
        }
        floating_node(::supdef::token_loc&& loc, const std::u32string& val)
            : node(std::move(loc))
            , expression_node()
            , m_val()
        {
            std::string str;
            for (char32_t c : val)
            {
                const auto nval = ::supdef::unicode::numeric_value<long long>(c);
                if (!nval.has_value())
                    str.push_back((char)c);
                else if ((long long)'0' + *nval < (long long)std::numeric_limits<char>::min() ||
                         (long long)'0' + *nval > (long long)std::numeric_limits<char>::max())
                    throw std::runtime_error("Invalid character in floating literal");
                else
                    str.push_back((char)((long long)'0' + *nval));
            }
            m_val = ::supdef::bigfloat(std::move(str));
        }

        const ::supdef::bigfloat& val() const
        {
            return m_val;
        }

        virtual kind node_kind() const override
        {
            return kind::floating;
        }

    private:
        ::supdef::bigfloat m_val;
    };
}