#ifndef TYPES_HPP
#define TYPES_HPP

#include <concepts>
#include <type_traits>
#include <source_location>
#include <filesystem>
#include <cstddef>
#include <cstdint>
#include <boost/multiprecision/gmp.hpp>

#include <experimental/scope>

#include <unicode.hpp>

namespace stdfs = ::std::filesystem;
namespace stdx = ::std::experimental;
namespace stdext = ::__gnu_cxx;

namespace supdef
{
    //struct location
    //{
    //    std::shared_ptr<const stdfs::path> filename;
    //    size_t line;
    //    size_t column;
    //    size_t off;
    //    size_t len;
    //};

    using bigint = boost::multiprecision::mpz_int;
    using bigfloat = boost::multiprecision::mpf_float;
}

#endif
