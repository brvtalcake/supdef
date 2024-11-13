#ifndef TYPES_HPP
#define TYPES_HPP

#include <concepts>
#include <type_traits>
#include <source_location>
#include <cstddef>
#include <cstdint>

namespace supdef
{
    enum direction : unsigned char
    {
        DIR_INPUT  = 1 << 0,
        DIR_OUTPUT = 1 << 1,
        DIR_INOUT  = DIR_INPUT | DIR_OUTPUT
    };

    template <enum direction Dir>
    struct u8_cstr
    {
        using type = void;
    };

    template <>
    struct u8_cstr<DIR_INPUT>
    {
        using type = const char*;
    };

    template <>
    struct u8_cstr<DIR_OUTPUT>
    {
        using type = char*;
    };

    template <>
    struct u8_cstr<DIR_INOUT>
    {
        using type = char*;
    };

    template <enum direction Dir>
    using u8_cstr_t = typename u8_cstr<Dir>::type;

    struct location
    {
        const char* filename;
        size_t line;
        size_t column;
        size_t offset;
    };
}

#endif
