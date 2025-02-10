#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP

#include <types.hpp>
#include <unicode.hpp>
#include <file.hpp>
#include <tokenizer.hpp>

#include <variant>
#include <string>
#include <optional>
#include <vector>
#include <map>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <algorithm>
#include <iterator>
#include <memory>
#include <utility>

#include <boost/multiprecision/gmp.hpp>
#include <boost/multiprecision/mpfr.hpp>

namespace supdef
{
    namespace interpreter
    {
        /* using sdinteger = bigint;
        using sdfloat   = bigfloat;
        using sdstring  = std::u32string;
        using sdtokens  = std::list<token>;

        using sdatom = std::variant<
            sdinteger,  // integer
            sdfloat,    // float
            sdstring,   // string
            sdtokens    // tokens
        >; */

        class basic_templated_block
        {
            public:
                virtual ~basic_templated_block() = default;
                
                virtual std::u32string to_string(
                    const std::vector<token>& args
                ) const = 0;
            
            protected:
                basic_templated_block() = default;

                std::vector<token> m_template;
        };
    }
}

#endif
