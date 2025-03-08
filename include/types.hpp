#ifndef TYPES_HPP
#define TYPES_HPP

#include <version.hpp>

#include <concepts>
#include <type_traits>
#include <source_location>
#include <filesystem>
#include <ranges>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <algorithm>
#include <execution>
#include <cstddef>
#include <cstdint>

#include <boost/preprocessor/config/config.hpp>
static_assert(
    BOOST_PP_IS_STANDARD(),
    "supdef requires a standard-compliant preprocessor"
);
static_assert(
    BOOST_PP_VARIADICS,
    "supdef requires variadic macro support"
);

#include <boost/multiprecision/gmp.hpp>

#include <boost/utility/identity_type.hpp>

#include <boost/stl_interfaces/iterator_interface.hpp>
#include <boost/stl_interfaces/reverse_iterator.hpp>
#include <boost/stl_interfaces/sequence_container_interface.hpp>
#include <boost/stl_interfaces/view_adaptor.hpp>
#include <boost/stl_interfaces/view_interface.hpp>

#include <boost/smart_ptr/local_shared_ptr.hpp>
#include <boost/smart_ptr/make_local_shared.hpp>

#include <experimental/scope>

#include <detail/xxhash.hpp>

#define DO_PRAGMA(x) _Pragma (#x)
#define TODO(...) DO_PRAGMA(message ("TODO - " #__VA_ARGS__))

namespace stdexec = std::execution;
namespace stdfs = ::std::filesystem;
namespace stdranges = ::std::ranges;
namespace stdviews = ::std::views;
namespace stdx = ::std::experimental;
namespace stdext = ::__gnu_cxx;

namespace booststl = ::boost::stl_interfaces;
namespace boostmp  = ::boost::multiprecision;

#undef  PACKED_STRUCT
#undef  PACKED_CLASS
#undef  PACKED_UNION
#undef  PACKED_ENUM
#ifdef __has_cpp_attribute
# if __has_cpp_attribute(__packed__) && 0
#  define PACKED_STRUCT(name) struct [[__packed__]] name
#  define PACKED_CLASS(name) class [[__packed__]] name
#  define PACKED_UNION(name) union [[__packed__]] name
#  define PACKED_ENUM(name) enum [[__packed__]] name
# endif
#endif
#if !defined(PACKED_STRUCT) && defined(__has_attribute)
# if __has_attribute(__packed__)
#  define PACKED_STRUCT(name) struct [[__gnu__::__packed__]] name
#  define PACKED_CLASS(name) class [[__gnu__::__packed__]] name
#  define PACKED_UNION(name) union [[__gnu__::__packed__]] name
#  define PACKED_ENUM(name) enum [[__gnu__::__packed__]] name
# endif
#endif
#if !defined(PACKED_STRUCT)
# error "no PACKED_{STRUCT,CLASS,...} macros defined"
#endif

namespace supdef
{
    template <class... Ts>
    struct overloaded : Ts...
    {
        using Ts::operator()...;
    };
    template <class... Ts>
    overloaded(Ts...) -> overloaded<Ts...>;

    template <typename...>
    struct select_overload;

    template <typename RetT, typename... ArgsT>
    struct select_overload<RetT(ArgsT...)>
    {
        using type = RetT(*)(ArgsT...);

        constexpr select_overload(type fn) noexcept
            : fn(fn)
        {
        }

        constexpr select_overload& operator=(type fn) noexcept
        {
            this->fn = fn;
            return *this;
        }

        constexpr operator type() const noexcept
        {
            return fn;
        }

        constexpr type operator()() const noexcept
        {
            return fn;
        }

        type fn;
    };

    template <typename RetT, typename... ArgsT>
    struct select_overload<RetT(*)(ArgsT...)>
        : select_overload<RetT(ArgsT...)>
    {
        using select_overload<RetT(ArgsT...)>::select_overload;
        using select_overload<RetT(ArgsT...)>::operator=;
    };

    template <typename RetT, typename... ArgsT>
    struct select_overload<RetT(ArgsT...) noexcept>
    {
        using type = RetT(*)(ArgsT...) noexcept;

        constexpr select_overload(type fn) noexcept
            : fn(fn)
        {
        }

        constexpr select_overload& operator=(type fn) noexcept
        {
            this->fn = fn;
            return *this;
        }
        
        constexpr operator type() const noexcept
        {
            return fn;
        }

        constexpr type operator()() const noexcept
        {
            return fn;
        }
    };

    template <typename RetT, typename... ArgsT>
    struct select_overload<RetT(*)(ArgsT...) noexcept>
        : select_overload<RetT(ArgsT...) noexcept>
    {
        using select_overload<RetT(ArgsT...) noexcept>::select_overload;
        using select_overload<RetT(ArgsT...) noexcept>::operator=;
    };

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

#undef  FWD_AUTO
#define FWD_AUTO(...) ::std::forward<std::remove_reference_t<decltype(__VA_ARGS__)>>(__VA_ARGS__)

#if SUPDEF_MULTITHREADED
    template <typename T>
    using shared_ptr = std::shared_ptr<T>;
#else
    template <typename T>
    using shared_ptr = boost::local_shared_ptr<T>;
#endif

    namespace detail
    {
        template <typename T, typename... Args>
        consteval bool make_shared_is_noexcept()
        {
            return static_cast<bool>(
                noexcept(
#if SUPDEF_MULTITHREADED
                    std::make_shared<T>(std::declval<Args>()...)
#else
                    boost::make_local_shared<T>(std::declval<Args>()...)
#endif
                )
            );
        }

        template <typename T, typename U>
        consteval bool ptr_cast_is_noexcept(std::string_view cast)
        {
            if constexpr (cast == "static")
            {
                return static_cast<bool>(
                    noexcept(
#if SUPDEF_MULTITHREADED
                        std::static_pointer_cast<T>(std::declval<U>())
#else
                        boost::static_pointer_cast<T>(std::declval<U>())
#endif
                    )
                );
            }
            else if constexpr (cast == "dynamic")
            {
                return static_cast<bool>(
                    noexcept(
#if SUPDEF_MULTITHREADED
                        std::dynamic_pointer_cast<T>(std::declval<U>())
#else
                        boost::dynamic_pointer_cast<T>(std::declval<U>())
#endif
                    )
                );
            }
            else if constexpr (cast == "const")
            {
                return static_cast<bool>(
                    noexcept(
#if SUPDEF_MULTITHREADED
                        std::const_pointer_cast<T>(std::declval<U>())
#else
                        boost::const_pointer_cast<T>(std::declval<U>())
#endif
                    )
                );
            }
            else if constexpr (cast == "reinterpret")
            {
                return static_cast<bool>(
                    noexcept(
#if SUPDEF_MULTITHREADED
                        std::reinterpret_pointer_cast<T>(std::declval<U>())
#else
                        boost::reinterpret_pointer_cast<T>(std::declval<U>())
#endif
                    )
                );
            }
            else
            {
                return false;
            }
        }
    }

    // make_shared
    template <typename T, typename... Args>
    static inline ::supdef::shared_ptr<T> make_shared(Args&&... args)
        noexcept(
            ::supdef::detail::make_shared_is_noexcept<T, Args...>()
        )
    {
#if SUPDEF_MULTITHREADED
        return std::make_shared<T>(std::forward<Args>(args)...);
#else
        return boost::make_local_shared<T>(std::forward<Args>(args)...);
#endif
    }

    template <typename T>
    static inline ::supdef::shared_ptr<T> static_pointer_cast(
        auto&& ptr
    ) noexcept(
        ::supdef::detail::ptr_cast_is_noexcept<T, decltype(ptr)>("static")
    ) {
#if SUPDEF_MULTITHREADED
        return std::static_pointer_cast<T>(FWD_AUTO(ptr));
#else
        return boost::static_pointer_cast<T>(FWD_AUTO(ptr));
#endif
    }

    template <typename T>
    static inline ::supdef::shared_ptr<T> dynamic_pointer_cast(
        auto&& ptr
    ) noexcept(
        ::supdef::detail::ptr_cast_is_noexcept<T, decltype(ptr)>("dynamic")
    ) {
#if SUPDEF_MULTITHREADED
        return std::dynamic_pointer_cast<T>(FWD_AUTO(ptr));
#else
        return boost::dynamic_pointer_cast<T>(FWD_AUTO(ptr));
#endif
    }

    template <typename T>
    static inline ::supdef::shared_ptr<T> const_pointer_cast(
        auto&& ptr
    ) noexcept(
        ::supdef::detail::ptr_cast_is_noexcept<T, decltype(ptr)>("const")
    ) {
#if SUPDEF_MULTITHREADED
        return std::const_pointer_cast<T>(FWD_AUTO(ptr));
#else
        return boost::const_pointer_cast<T>(FWD_AUTO(ptr));
#endif
    }

    template <typename T>
    static inline ::supdef::shared_ptr<T> reinterpret_pointer_cast(
        auto&& ptr
    ) noexcept(
        ::supdef::detail::ptr_cast_is_noexcept<T, decltype(ptr)>("reinterpret")
    ) {
#if SUPDEF_MULTITHREADED
        return std::reinterpret_pointer_cast<T>(FWD_AUTO(ptr));
#else
        return boost::reinterpret_pointer_cast<T>(FWD_AUTO(ptr));
#endif
    }
}

#endif
