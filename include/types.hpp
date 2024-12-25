#ifndef TYPES_HPP
#define TYPES_HPP

#include <concepts>
#include <type_traits>
#include <source_location>
#include <filesystem>
#include <cstddef>
#include <cstdint>

namespace stdfs = std::filesystem;
namespace stdx = std::experimental;
namespace stdext = __gnu_cxx;

namespace supdef
{
    struct location
    {
        stdfs::path filename;
        size_t line;
        size_t column;
        size_t offset;
    };
}

#endif
