#ifndef DIRECTIVES_HPP
#define DIRECTIVES_HPP

#include <types.hpp>
#include <unicode.hpp>
#include <file.hpp>
#include <tokenizer.hpp>

#include <variant>
#include <string>
#include <optional>

#if __cpp_explicit_this_parameter < 202110L
    #error "explicit this parameter is required but not available"
#endif

namespace supdef
{
    class ast_node
    {
    public:
        virtual ~ast_node() = default;
        virtual std::u32string to_string() const = 0;
    };

    // TODO
}

#endif
