#ifndef DIRECTIVES_HPP
#define DIRECTIVES_HPP

#include <types.hpp>
#include <unicode.hpp>
#include <file.hpp>
#include <tokenizer.hpp>

#include <variant>
#include <string>
#include <optional>
#include <vector>
#include <map>
#include <utility>

#include <boost/multiprecision/gmp.hpp>

#if __cpp_explicit_this_parameter < 202110L
    #error "explicit this parameter is required but not available"
#endif

namespace supdef
{
    // TODO: class for supdef and runnable directives

    // { currently invoked supdef name, variable name }
    using supdef_varname = std::pair<std::u32string, std::u32string>;

    using supdef_variable = std::variant<
        boost::multiprecision::gmp_int,
        boost::multiprecision::gmp_float,
        std::u32string,
        std::vector<std::u32string>
    >;

    class supdef_interpreter
    {
    private:
        std::map<supdef_varname, supdef_variable> m_variables;
    };
}

#endif
