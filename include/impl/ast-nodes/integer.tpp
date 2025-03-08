#include <string>
#include <iostream>
#include <sstream>

namespace supdef::ast
{
    class integer_node final
        : public virtual node
        , public expression_node
    {
    public:
        integer_node(::supdef::token_loc&& loc, ::supdef::bigint&& val)
            : node(std::move(loc))
            , expression_node()
            , m_val(std::move(val))
        {
        }
        integer_node(::supdef::token_loc&& loc, const std::u32string& val)
            : node(std::move(loc))
            , expression_node()
            , m_val()
        {
            std::string str;
            for (char32_t c : val)
            {
                // TODO: maybe already checked while tokenizing ?
                const auto nval = ::supdef::unicode::numeric_value<long long>(c);
                if (!nval.has_value() ||
                    (long long)'0' + *nval < (long long)std::numeric_limits<char>::min() ||
                    (long long)'0' + *nval > (long long)std::numeric_limits<char>::max())
                    throw std::runtime_error("Invalid character in integer literal");
                str.push_back((char)((long long)'0' + nval));
            }
            m_val = ::supdef::bigint(std::move(str));
        }

        const ::supdef::bigint& val() const
        {
            return m_val;
        }

        virtual kind node_kind() const override
        {
            return kind::integer;
        }

    private:
        ::supdef::bigint m_val;
    };
}