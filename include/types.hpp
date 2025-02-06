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
#include <execution>
#include <cstddef>
#include <cstdint>

#include <boost/multiprecision/gmp.hpp>

#include <boost/utility/identity_type.hpp>

#include <boost/stl_interfaces/iterator_interface.hpp>
#include <boost/stl_interfaces/reverse_iterator.hpp>
#include <boost/stl_interfaces/sequence_container_interface.hpp>
#include <boost/stl_interfaces/view_adaptor.hpp>
#include <boost/stl_interfaces/view_interface.hpp>

#include <experimental/scope>

#include <detail/xxhash.hpp>

namespace stdexec = std::execution;
namespace stdfs = ::std::filesystem;
namespace stdranges = ::std::ranges;
namespace stdviews = ::std::views;
namespace stdx = ::std::experimental;
namespace stdext = ::__gnu_cxx;

namespace booststl = ::boost::stl_interfaces;
namespace boostmp  = ::boost::multiprecision;

namespace supdef
{
    using bigint = boostmp::mpz_int;
    using bigfloat = boostmp::mpf_float;
    using bigdecimal = boostmp::mpq_rational;
    using bigcomplex = boostmp::mpc_complex;

    template <
        typename Key, typename T, typename... Rest
    >
    using umap = std::unordered_map<
        Key, T, ::supdef::detail::xxhash<Key, 64>, Rest...
    >;
    static_assert(
        std::is_same<
            umap<std::u32string, int>,
            std::unordered_map<
                std::u32string, int,
                ::supdef::detail::xxhash<std::u32string, 64>
            >
        >::value
    );

    template <
        typename Key, typename T, typename... Rest
    >
    using umultimap = std::unordered_multimap<
        Key, T, ::supdef::detail::xxhash<Key, 64>, Rest...
    >;
    static_assert(
        std::is_same<
            umultimap<std::u32string, int>,
            std::unordered_multimap<
                std::u32string, int,
                ::supdef::detail::xxhash<std::u32string, 64>
            >
        >::value
    );

    template <typename T, typename... Rest>
    using uset = std::unordered_set<
        T, ::supdef::detail::xxhash<T, 64>, Rest...
    >;
    static_assert(
        std::is_same<
            uset<std::u32string>,
            std::unordered_set<
                std::u32string,
                ::supdef::detail::xxhash<std::u32string, 64>
            >
        >::value
    );

    template <typename T, typename... Rest>
    using umultiset = std::unordered_multiset<
        T, ::supdef::detail::xxhash<T, 64>, Rest...
    >;
    static_assert(
        std::is_same<
            umultiset<std::u32string>,
            std::unordered_multiset<
                std::u32string,
                ::supdef::detail::xxhash<std::u32string, 64>
            >
        >::value
    );
}

#endif
