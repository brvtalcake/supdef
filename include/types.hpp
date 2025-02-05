#ifndef TYPES_HPP
#define TYPES_HPP

#include <concepts>
#include <type_traits>
#include <source_location>
#include <filesystem>
#include <ranges>
#include <views>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <boost/multiprecision/gmp.hpp>
#include <boost/msm/back/state_machine.hpp>

#include <experimental/scope>

#include <detail/xxhash.hpp>

namespace stdfs = ::std::filesystem;
namespace stdranges = ::std::ranges;
namespace stdviews = ::std::views;
namespace stdx = ::std::experimental;
namespace stdext = ::__gnu_cxx;

namespace supdef
{
    using bigint = boost::multiprecision::mpz_int;
    using bigfloat = boost::multiprecision::mpf_float;
    using bigdecimal = boost::multiprecision::mpq_rational;
    using bigcomplex = boost::multiprecision::mpc_complex;

    template <
        typename Key, typename T,
        typename Pred = std::equal_to<Key>,
        typename Alloc = std::allocator<std::pair<const Key, T>>
    >
    using umap = std::unordered_map<
        Key, T, ::supdef::detail::xxhash<Key, 64>, Pred, Alloc
    >;

    template <
        typename Key, typename T,
        typename Pred = std::equal_to<Key>,
        typename Alloc = std::allocator<std::pair<const Key, T>>
    >
    using umultimap = std::unordered_multimap<
        Key, T, ::supdef::detail::xxhash<Key, 64>, Pred, Alloc
    >;

    template <
        typename T,
        typename Pred = std::equal_to<T>,
        typename Alloc = std::allocator<T>
    >
    using uset = std::unordered_set<
        T, ::supdef::detail::xxhash<T, 64>, Pred, Alloc
    >;

    template <
        typename T,
        typename Pred = std::equal_to<T>,
        typename Alloc = std::allocator<T>
    >
    using umultiset = std::unordered_multiset<
        T, ::supdef::detail::xxhash<T, 64>, Pred, Alloc
    >;
}

#endif
