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

#include <boost/hof.hpp>

#include <boost/pool/pool.hpp>

#include <boost/mpl/for_each.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/transform.hpp>

#include <boost/hana.hpp>

#include <boost/hof.hpp>

#include <boost/mp11.hpp>

#include <boost/functional.hpp>

#include <boost/function_types/result_type.hpp>
#include <boost/function_types/function_type.hpp>
#include <boost/function_types/parameter_types.hpp>
#include <boost/function_types/function_arity.hpp>

#include <boost/preprocessor/cat.hpp>

#include <experimental/scope>

#define DO_PRAGMA(x) _Pragma (#x)
#define TODO(...) DO_PRAGMA(message ("TODO - " #__VA_ARGS__))

namespace stdexec = std::execution;
namespace stdfs = ::std::filesystem;
namespace stdranges = ::std::ranges;
namespace stdviews = ::std::views;
namespace stdx = ::std::experimental;
namespace gnucxx = ::__gnu_cxx;

namespace booststl = ::boost::stl_interfaces;
namespace boostmp  = ::boost::multiprecision;
namespace functional = ::boost::functional;
namespace fn_types = ::boost::function_types;
namespace mpl = ::boost::mpl;
namespace hana = ::boost::hana;
namespace mp11 = ::boost::mp11;
namespace hof = ::boost::hof;

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

#undef  ATTRIBUTE_UNINITIALIZED
#define ATTRIBUTE_UNINITIALIZED [[__gnu__::__uninitialized__]]

#include <detail/xxhash.hpp>
#include <detail/ckd_arith.hpp>

//#undef  is_expression_valid
//#undef  is_expression_valid_v
//#define is_expression_valid(...) std::bool_constant<requires { (__VA_ARGS__); }>
//#define is_expression_valid_v(...) (is_expression_valid(__VA_ARGS__))::value
//
//static_assert(
//    is_expression_valid(1 + 1)::value,
//    "1 + 1 is not a valid expression"
//);
//static_assert(
//    !is_expression_valid(1 + std::pair{1, 2})::value,
//    "1 + std::pair{1, 2} is a valid expression"
//);
    

namespace supdef
{
    template<typename T, typename G = decltype([](){})>
    consteval bool is_type_complete()
    {
        return requires { typename std::void_t<decltype(sizeof(T))>; };
    }

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

        type fn;
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
#define FWD_AUTO(...) ::std::forward<decltype(__VA_ARGS__)>(__VA_ARGS__)

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

        template <typename T, typename Alloc, typename... Args>
        consteval bool allocate_shared_is_noexcept()
        {
            return static_cast<bool>(
                noexcept(
#if SUPDEF_MULTITHREADED
                    std::allocate_shared<T>(std::declval<Alloc>(), std::declval<Args>()...)
#else
                    boost::allocate_local_shared<T>(std::declval<Alloc>(), std::declval<Args>()...)
#endif
                )
            );
        }

        template <typename T, typename U>
        consteval bool ptr_cast_is_noexcept(std::string_view cast)
        {
            if (cast == "static")
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
            else if (cast == "dynamic")
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
            else if (cast == "const")
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
            else if (cast == "reinterpret")
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

    template <typename T, typename Alloc, typename... Args>
    static inline ::supdef::shared_ptr<T> allocate_shared(
        Alloc&& alloc, Args&&... args
    ) noexcept(
        ::supdef::detail::allocate_shared_is_noexcept<T, Alloc, Args...>()
    ) {
#if SUPDEF_MULTITHREADED
        return std::allocate_shared<T>(std::forward<Alloc>(alloc), std::forward<Args>(args)...);
#else
        return boost::allocate_local_shared<T>(std::forward<Alloc>(alloc), std::forward<Args>(args)...);
#endif
    }


    template <typename T>
    static inline ::supdef::shared_ptr<T> static_pointer_cast(
        auto&& ptr
    ) noexcept(
        (bool) ([] consteval { return ::supdef::detail::ptr_cast_is_noexcept<T, decltype(ptr)>("static"); }())
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
        (bool) ([] consteval { return ::supdef::detail::ptr_cast_is_noexcept<T, decltype(ptr)>("dynamic"); }())
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
        (bool) ([] consteval { return ::supdef::detail::ptr_cast_is_noexcept<T, decltype(ptr)>("const"); }())
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
        (bool) ([] consteval { return ::supdef::detail::ptr_cast_is_noexcept<T, decltype(ptr)>("reinterpret"); }())
    ) {
#if SUPDEF_MULTITHREADED
        return std::reinterpret_pointer_cast<T>(FWD_AUTO(ptr));
#else
        return boost::reinterpret_pointer_cast<T>(FWD_AUTO(ptr));
#endif
    }

    struct slice : stdranges::range_adaptor_closure<slice>
    {
        size_t start = 0;
        size_t end = std::string_view::npos;

        static constexpr bool _S_has_simple_call_op = true;
    
        constexpr std::string_view operator()(std::string_view sv) const
        {
            return sv.substr(this->start, this->end - this->start);
        }
    };

    template <typename RngT>
    concept can_prev = requires(stdranges::const_iterator_t<RngT> it, stdranges::const_sentinel_t<RngT> s) {
        { stdranges::prev(s) } -> std::equality_comparable_with<stdranges::const_iterator_t<RngT>>;
    };

    template <stdranges::view R>
        requires can_prev<R>
    struct drop_last_view : stdranges::view_interface<drop_last_view<R>>
    {
        using value_type = stdranges::range_value_t<R>;
        using reference = stdranges::range_reference_t<R>;
        using const_reference = stdranges::range_reference_t<R>;
        using iterator = stdranges::const_iterator_t<R>;
        using sentinel = stdranges::const_iterator_t<R>;
        using size_type = stdranges::range_size_t<R>;
        using difference_type = stdranges::range_difference_t<R>;

    private:
        R m_base;
        sentinel m_stop_at;
        size_type m_remove;

    public:
        drop_last_view() = default;

        constexpr drop_last_view(R base, size_type n)
            : m_base(std::move(base))
            , m_stop_at(stdranges::prev(stdranges::cend(m_base), n))
            , m_remove(n)
        {
            assert(n > 0);
            if constexpr (stdranges::sized_range<R>)
                assert(n <= stdranges::size(m_base));
        }

        constexpr drop_last_view(drop_last_view &&) = default;
        constexpr drop_last_view &operator=(drop_last_view &&) = default;

        constexpr iterator begin()
        {
            return stdranges::cbegin(m_base);
        }

        constexpr iterator begin() const
            requires stdranges::range<const R>
        {
            return stdranges::cbegin(m_base);
        }

        constexpr sentinel end()
        {
            return m_stop_at;
        }

        constexpr sentinel end() const
            requires stdranges::range<const R>
        {
            return m_stop_at;
        }

        constexpr size_type size() const
            requires stdranges::sized_range<R>
        {
            return stdranges::size(m_base) - m_remove;
        }
    };

    struct drop_last_closure : stdranges::range_adaptor_closure<drop_last_closure>
    {
    private:
        struct drop_last_partial_apply_closure : stdranges::range_adaptor_closure<drop_last_partial_apply_closure>
        {
            const drop_last_closure* ptr;
            size_t n;

            template <stdranges::viewable_range RngT>
                requires can_prev<RngT>
            constexpr auto operator()(RngT&& rng) const
            {
                return (*this->ptr)(std::forward<RngT>(rng), checked_cast<stdranges::range_size_t<RngT>>(this->n));
            }
        };

    public:
        /* static constexpr bool _S_has_simple_call_op = true; */

        template <stdranges::viewable_range RngT>
            requires can_prev<RngT>
        constexpr auto operator()(RngT&& rng, stdranges::range_size_t<RngT> n) const
        {
            typedef RngT range_type;
            typedef stdranges::range_value_t<range_type> value_type;

            constexpr auto wrapper = [](auto &&rng) -> stdviews::all_t<range_type> {
                return stdviews::all(std::forward<range_type>(rng));
            };

            if constexpr (stdranges::sized_range<range_type>)
            {
                auto sz = stdranges::size(rng);
                typedef decltype(sz) size_type;

                if (n >= sz) [[__unlikely__]]
                    return stdranges::take_view{wrapper(std::forward<range_type>(rng)), 0};
                else
                {
                    stdranges::range_difference_t<range_type> diff;
                    if (!::supdef::checked_sub(diff, sz, n))
                        throw std::overflow_error("overflow in drop_last");
                    return stdranges::take_view{
                        wrapper(std::forward<range_type>(rng)), diff
                    };
                }
            }
            else
                return drop_last_view{wrapper(std::forward<range_type>(rng)), n};
        }

        constexpr drop_last_partial_apply_closure operator()(size_t n) const
        {
            return { .ptr = this, .n = n };
        }
    };

    inline constexpr drop_last_closure drop_last;
}

template <typename R>
inline constexpr bool stdranges::disable_sized_range<supdef::drop_last_view<R>> = stdranges::disable_sized_range<R>;

#if 0
template <typename R>
inline constexpr bool stdranges::enable_borrowed_range<supdef::drop_last_view<R>> = stdranges::enable_borrowed_range<R>;
#endif

#endif
