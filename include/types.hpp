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

#include <boost/pfr.hpp>

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
namespace pool = ::boost::pool;
namespace pfr = ::boost::pfr;

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
    template <typename T>
    concept reflectable = pfr::is_implicitly_reflectable_v<std::remove_reference_t<T>, void>;

    template <typename FromT, typename ToT>
    struct copy_refs_from
    {
        using type = std::remove_reference_t<ToT>;
    };

    template <typename FromT, typename ToT>
    struct copy_refs_from<FromT&, ToT>
    {
        using type = std::add_lvalue_reference_t<
            std::remove_reference_t<ToT>
        >;
    };

    template <typename FromT, typename ToT>
    struct copy_refs_from<FromT&&, ToT>
    {
        using type = std::add_rvalue_reference_t<
            std::remove_reference_t<ToT>
        >;
    };

    template <typename FromT, typename ToT>
    struct copy_cv_from
    {
        using type = std::remove_cv_t<ToT>;
    };

    template <typename FromT, typename ToT>
    struct copy_cv_from<FromT const, ToT>
    {
        using type = std::add_const_t<
            std::remove_cv_t<ToT>
        >;
    };

    template <typename FromT, typename ToT>
    struct copy_cv_from<FromT volatile, ToT>
    {
        using type = std::add_volatile_t<
            std::remove_cv_t<ToT>
        >;
    };

    template <typename FromT, typename ToT>
    struct copy_cv_from<FromT const volatile, ToT>
    {
        using type = std::add_cv_t<
            std::remove_cv_t<ToT>
        >;
    };

    template <typename FromT, typename ToT>
    struct copy_cv_from<FromT, ToT&>
    {
        using type = std::add_lvalue_reference_t<
            typename copy_cv_from<FromT, ToT>::type
        >;
    };

    template <typename FromT, typename ToT>
    struct copy_cv_from<FromT, ToT&&>
    {
        using type = std::add_rvalue_reference_t<
            typename copy_cv_from<FromT, ToT>::type
        >;
    };

    template <typename FromT, typename ToT>
    struct copy_cv_from<FromT&, ToT>
        : public copy_cv_from<FromT, ToT>
    {
    };

    template <typename FromT, typename ToT>
    struct copy_cv_from<FromT&&, ToT>
        : public copy_cv_from<FromT, ToT>
    {
    };

    template <typename FromT, typename ToT>
    struct copy_quals_from
        : public copy_cv_from<
            FromT, typename copy_refs_from<
                FromT, ToT
            >::type
        >
    {
    };

    template <typename FromT, typename ToT>
    using copy_refs_from_t = typename copy_refs_from<FromT, ToT>::type;
    
    template <typename FromT, typename ToT>
    using copy_cv_from_t = typename copy_cv_from<FromT, ToT>::type;

    template <typename FromT, typename ToT>
    using copy_quals_from_t = typename copy_quals_from<FromT, ToT>::type;

    namespace detail
    {
        template <size_t, typename...>
        struct nth_type_impl;

        template <size_t N, typename T, typename... Ts>
        struct nth_type_impl<N, T, Ts...>
        {
            using type = typename nth_type_impl<N - 1, Ts...>::type;
        };

        template <typename T, typename... Ts>
        struct nth_type_impl<0, T, Ts...>
        {
            using type = T;
        };

        template <typename T>
        struct nth_type_impl<0, T>
        {
            using type = T;
        };
    }

    template <size_t N, typename... Ts>
        requires (N < sizeof...(Ts))
    struct nth_type
    {
        using type = typename nth_type_impl<N, Ts...>::type;
    };

    template <size_t N, typename... Ts>
    using nth_type_t = typename nth_type<N, Ts...>::type;

    template <size_t, typename>
    struct prepend_to_index_sequence;

    template <size_t N, size_t... Is>
    struct prepend_to_index_sequence<N, std::index_sequence<Is...>>
    {
        using type = std::index_sequence<N, Is...>;
    };

    template <size_t N, typename Is>
    using prepend_to_index_sequence_t = typename prepend_to_index_sequence<N, Is>::type;

    namespace detail
    {
        template <size_t Cur, size_t N, typename... Ts>
        struct index_sequence_for_except_impl;

        template <size_t Cur, size_t N, typename T, typename... Ts>
        struct index_sequence_for_except_impl<Cur, N, T, Ts...>
        {
            using type = typename prepend_to_index_sequence<
                Cur, typename index_sequence_for_except_impl<Cur + 1, N, Ts...>::type
            >::type;
        };

        template <size_t N, typename T, typename... Ts>
        struct index_sequence_for_except_impl<N, N, T, Ts...>
        {
            using type = typename index_sequence_for_except_impl<N + 1, N, Ts...>::type;
        };

        template <size_t Cur, size_t N>
        struct index_sequence_for_except_impl<Cur, N>
        {
            using type = std::index_sequence<>;
        };
    }

    template <size_t N, typename... Ts>
    using index_sequence_for_except = typename
        detail::index_sequence_for_except_impl<
            0, N, Ts...
        >::type;
    
    namespace detail
    {
        template <typename>
        struct index_sequence_for_tuple_impl;

        template <typename... T>
        struct index_sequence_for_tuple_impl<std::tuple<T...>>
        {
            using type = std::index_sequence_for<T...>;
        };
    }
    
    template <typename T>
    using index_sequence_for_tuple = typename
        detail::index_sequence_for_tuple_impl<T>::type;

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

    using bigint_base = boostmp::mpz_int;
    using bigfloat_base = boostmp::mpfr_float;
    using bigdecimal_base = boostmp::mpq_rational;
    using bigcomplex_base = boostmp::mpfi_float;

    struct big_numbers_base
    {
        template <typename T, typename Self>
        constexpr T as(this Self&& self)
        {
            return static_cast<T>(self);
        }
    };

    struct bigint
        : public virtual big_numbers_base
        , public bigint_base
    {
        using bigint_base::bigint_base;
    };

    struct bigfloat
        : public virtual big_numbers_base
        , public bigfloat_base
    {
        using bigfloat_base::bigfloat_base;
    };

    struct bigdecimal
        : public virtual big_numbers_base
        , public bigdecimal_base
    {
        using bigdecimal_base::bigdecimal_base;
    };

    struct bigcomplex
        : public virtual big_numbers_base
        , public bigcomplex_base
    {
        using bigcomplex_base::bigcomplex_base;
    };

    // big numerator
    struct bignum : bigint
    {
        using bigint::bigint;

        constexpr bigdecimal operator/(const bigint& rhs) const
        {
            return bigdecimal(*this) / rhs;
        }
    };
    // big denominator
    struct bigdenom : bigint
    {
        using bigint::bigint;
    };

    template <char... Chars>
    consteval bigint operator""_bigint()
    {
        char str[] = {Chars..., '\0'};
        return bigint(std::string_view{str});
    }
    consteval bigint operator""_bigint(const char* str, size_t len)
    {
        return bigint(std::string_view{str, len});
    }

    template <char... Chars>
    consteval bigfloat operator""_bigfloat()
    {
        char str[] = {Chars..., '\0'};
        return bigfloat(std::string_view{str});
    }
    consteval bigfloat operator""_bigfloat(const char* str, size_t len)
    {
        return bigfloat(std::string_view{str, len});
    }

    template <char... Chars>
    consteval bigdecimal operator""_bigdec()
    {
        char str[] = {Chars..., '\0'};
        return bigdecimal(std::string_view{str});
    }
    consteval bigdecimal operator""_bigdec(const char* str, size_t len)
    {
        return bigdecimal(std::string_view{str, len});
    }

    template <char... Chars>
    consteval bigcomplex operator""_bigcomplex()
    {
        char str[] = {Chars..., '\0'};
        return bigcomplex(std::string_view{str});
    }
    consteval bigcomplex operator""_bigcomplex(const char* str, size_t len)
    {
        return bigcomplex(std::string_view{str, len});
    }

    template <char... Chars>
    consteval bignum operator""_bignum()
    {
        char str[] = {Chars..., '\0'};
        return bignum(std::string_view{str});
    }
    consteval bignum operator""_bignum(const char* str, size_t len)
    {
        return bignum(std::string_view{str, len});
    }

    template <char... Chars>
    consteval bigdenom operator""_bigdenom()
    {
        char str[] = {Chars..., '\0'};
        return bigdenom(std::string_view{str});
    }
    consteval bigdenom operator""_bigdenom(const char* str, size_t len)
    {
        return bigdenom(std::string_view{str, len});
    }

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
#define FWD_AUTO(...) (::std::forward<decltype(__VA_ARGS__)>(__VA_ARGS__))

#undef  AS_CONSTEVAL
#define AS_CONSTEVAL(...) (([] { return (__VA_ARGS__); })())

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
    static inline decltype(auto) make_shared(Args&&... args)
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
    static inline decltype(auto) allocate_shared(
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
    static inline decltype(auto) static_pointer_cast(
        auto&& ptr
    ) noexcept(
        AS_CONSTEVAL(::supdef::detail::ptr_cast_is_noexcept<T, decltype(ptr)>("static"))
    ) {
#if SUPDEF_MULTITHREADED
        return std::static_pointer_cast<T>(FWD_AUTO(ptr));
#else
        return boost::static_pointer_cast<T>(FWD_AUTO(ptr));
#endif
    }

    template <typename T>
    static inline decltype(auto) dynamic_pointer_cast(
        auto&& ptr
    ) noexcept(
        AS_CONSTEVAL(::supdef::detail::ptr_cast_is_noexcept<T, decltype(ptr)>("dynamic"))
    ) {
#if SUPDEF_MULTITHREADED
        return std::dynamic_pointer_cast<T>(FWD_AUTO(ptr));
#else
        return boost::dynamic_pointer_cast<T>(FWD_AUTO(ptr));
#endif
    }

    template <typename T>
    static inline decltype(auto) const_pointer_cast(
        auto&& ptr
    ) noexcept(
        AS_CONSTEVAL(::supdef::detail::ptr_cast_is_noexcept<T, decltype(ptr)>("const"))
    ) {
#if SUPDEF_MULTITHREADED
        return std::const_pointer_cast<T>(FWD_AUTO(ptr));
#else
        return boost::const_pointer_cast<T>(FWD_AUTO(ptr));
#endif
    }

    template <typename T>
    static inline decltype(auto) reinterpret_pointer_cast(
        auto&& ptr
    ) noexcept(
        AS_CONSTEVAL(::supdef::detail::ptr_cast_is_noexcept<T, decltype(ptr)>("reinterpret"))
    ) {
#if SUPDEF_MULTITHREADED
        return std::reinterpret_pointer_cast<T>(FWD_AUTO(ptr));
#else
        return boost::reinterpret_pointer_cast<T>(FWD_AUTO(ptr));
#endif
    }

    struct slice : public stdranges::range_adaptor_closure<slice>
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
    struct drop_last_view : public stdranges::view_interface<drop_last_view<R>>
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

    struct drop_last_closure : public stdranges::range_adaptor_closure<drop_last_closure>
    {
    private:
        struct drop_last_partial_apply_closure : public stdranges::range_adaptor_closure<drop_last_partial_apply_closure>
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

    using unused_t = decltype(std::ignore);

    template <typename T>
    concept has_boxed_type = std::is_array_v<T> || requires {
        typename std::pointer_traits<
            std::remove_cvref_t<T>
        >::element_type;
    };

    template <typename T>
        requires has_boxed_type<T>
    struct boxed_type
        : public std::type_identity<
            typename std::pointer_traits<
                std::remove_cvref_t<T>
            >::element_type
        >
    {
    };

    template <typename T>
    struct boxed_type<T[]>
        : public std::type_identity<T>
    {
    };

    template <typename T, size_t N>
    struct boxed_type<T[N]>
        : public std::type_identity<T>
    {
    };

    template <typename T>
    using boxed_type_t = typename boxed_type<T>::type;

}
namespace detail
{
    namespace test
    {
        consteval bool test_adl_static_pointer_cast()
        {
            using namespace supdef;

            struct A { char dummy; };
            struct B : A {};
            struct C : B {};

            auto a = make_shared<A>();

            auto b = dynamic_pointer_cast<B>(a);
        }
    }
}

template <typename R>
inline constexpr bool stdranges::disable_sized_range<supdef::drop_last_view<R>> = stdranges::disable_sized_range<R>;

#if 0
template <typename R>
inline constexpr bool stdranges::enable_borrowed_range<supdef::drop_last_view<R>> = stdranges::enable_borrowed_range<R>;
#endif

#include <test/copy_cvref_from.hpp>
#include <test/nth_type.hpp>
#include <test/prepend_to_is.hpp>
#include <test/index_sequence_for_XXX.hpp>
#include <test/boxed_type.hpp>

#include <detail/xxhash.hpp>
#include <detail/ckd_arith.hpp>
#include <detail/simple_map.hpp>
#include <detail/globals.hpp>

#endif // TYPES_HPP
