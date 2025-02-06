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
    // TODO: class for supdef and runnable directives

    // { currently invoked supdef name, variable name }
    using sdvarname = std::pair<std::u32string, std::u32string>;

    using sdatom = std::variant<
        bigint,                      // integer
        bigfloat,                    // float
        std::u32string               // string
    >;
    class sdcomposite;

    using sdvariable = std::variant<
        sdatom,
        sdcomposite
    >;

    class sdcomposite
    {
    public:
        enum class kind
        {
            tuple,   // (tokens, tokens, ...)
            list,    // [tokens, tokens, ...]
            set,     // {tokens, tokens, ...}
            map      // {tokens: tokens, tokens: tokens, ...}
        };
    private:
        using var_type = std::variant<
            std::vector<sdvariable>,
            std::list<sdvariable>,
            std::map<sdvariable, sdvariable>,
            uset<sdvariable>
        >;
        var_type m_variables;
        kind m_kind;
    };

    class supdef_interpreter_state
    {
    private:
        std::map<sdvarname, sdatom> m_variables;
    };
}

#endif
