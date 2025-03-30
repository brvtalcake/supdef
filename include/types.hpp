#ifndef TYPES_HPP
#define TYPES_HPP

#include <version.hpp>

#include <bits/stdc++.h>
#include <bits/extc++.h>
#include <execution>

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
#include <boost/multiprecision/mpfr.hpp>
#include <boost/multiprecision/mpfi.hpp>
#include <boost/multiprecision/mpc.hpp>
#include <boost/multiprecision/number.hpp>

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

#include <boost/mpl/vector.hpp>
#include <boost/mpl/contains.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/transform.hpp>

#include <boost/hana.hpp>

#include <boost/hof.hpp>

#include <boost/mp11.hpp>

#include <boost/functional.hpp>

#include <boost/function_types/result_type.hpp>
#include <boost/function_types/function_type.hpp>
#include <boost/function_types/parameter_types.hpp>
#include <boost/function_types/function_arity.hpp>

#include <boost/callable_traits.hpp>

#include <boost/preprocessor/cat.hpp>

#include <boost/pfr.hpp>

#include <boost/intrusive/hashtable.hpp>

#include <boost/endian/arithmetic.hpp>

#include <eve/eve.hpp>

#include <experimental/scope>

#define DO_PRAGMA(x) _Pragma (#x)
#define TODO(...) DO_PRAGMA(message ("TODO - " #__VA_ARGS__))

namespace stdexec = std::execution;
namespace stdfs = ::std::filesystem;
namespace stdranges = ::std::ranges;
namespace stdviews = ::std::views;
namespace stdx = ::std::experimental;
namespace gnucxx = ::__gnu_cxx;
namespace pbds = ::__gnu_pbds;

namespace booststl = ::boost::stl_interfaces;
namespace boostmp  = ::boost::multiprecision;
namespace functional = ::boost::functional;
namespace fn_types = ::boost::function_types;
namespace mpl = ::boost::mpl;
namespace hana = ::boost::hana;
namespace mp11 = ::boost::mp11;
namespace hof = ::boost::hof;
namespace pfr = ::boost::pfr;
namespace fn_traits = ::boost::callable_traits;
namespace intrusive = ::boost::intrusive;
namespace endian = ::boost::endian;

#undef  PACKED_STRUCT
#undef  PACKED_CLASS
#undef  PACKED_UNION
#undef  PACKED_ENUM
#undef  PACKED_ALIGNED_STRUCT
#undef  PACKED_ALIGNED_CLASS
#undef  PACKED_ALIGNED_UNION
#undef  PACKED_ALIGNED_ENUM

#define PACKED_STRUCT(name) struct ATTRIBUTE_PACKED name
#define PACKED_CLASS(name) class ATTRIBUTE_PACKED name
#define PACKED_UNION(name) union ATTRIBUTE_PACKED name
#define PACKED_ENUM(name) enum ATTRIBUTE_PACKED name
#define PACKED_ALIGNED_STRUCT(name, align) struct ATTRIBUTE_PACKED ATTRIBUTE_ALIGNED(align) name
#define PACKED_ALIGNED_CLASS(name, align) class ATTRIBUTE_PACKED ATTRIBUTE_ALIGNED(align) name
#define PACKED_ALIGNED_UNION(name, align) union ATTRIBUTE_PACKED ATTRIBUTE_ALIGNED(align) name
#define PACKED_ALIGNED_ENUM(name, align) enum ATTRIBUTE_PACKED ATTRIBUTE_ALIGNED(align) name

#undef  ATTRIBUTE_PACKED
#undef  ATTRIBUTE_ALIGNED

#ifdef __has_cpp_attribute
#  if __has_cpp_attribute(__packed__)
#    define ATTRIBUTE_PACKED [[__packed__]]
#  elif __has_cpp_attribute(__gnu__::__packed__)
#    define ATTRIBUTE_PACKED [[__gnu__::__packed__]]
#  endif
#elif defined(__has_attribute)
#  if __has_attribute(__packed__)
#    define ATTRIBUTE_PACKED __attribute__((__packed__))
#  endif
#endif

#ifdef __has_cpp_attribute
#  if __has_cpp_attribute(__aligned__)
#    define ATTRIBUTE_ALIGNED(align) [[__aligned__(align)]]
#  elif __has_cpp_attribute(__gnu__::__aligned__)
#    define ATTRIBUTE_ALIGNED(align) [[__gnu__::__aligned__(align)]]
#  endif
#elif defined(__has_attribute)
#  if __has_attribute(__aligned__)
#    define ATTRIBUTE_ALIGNED(align) __attribute__((__aligned__(align)))
#  endif
#else
#  define ATTRIBUTE_ALIGNED(align) alignas(align)
#endif

#if !defined(ATTRIBUTE_PACKED)
#  error "no __packed__ attribute"
#endif

#if !defined(ATTRIBUTE_ALIGNED)
#  error "no __aligned__ attribute"
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

namespace supdef { namespace detail { } }

namespace sd = ::supdef;

#include <detail/ckd_arith_fwd.hpp>

namespace supdef
{
    template <bool Cond, typename T>
    struct add_const_if
    {
        using type = T;
    };

    template <typename T>
    struct add_const_if<true, T>
    {
        using type = std::add_const_t<T>;
    };

    template <bool Cond, typename T>
    struct add_volatile_if
    {
        using type = T;
    };

    template <typename T>
    struct add_volatile_if<true, T>
    {
        using type = std::add_volatile_t<T>;
    };

    template <bool Cond, typename T>
    using add_const_if_t = typename add_const_if<Cond, T>::type;

    template <bool Cond, typename T>
    using add_volatile_if_t = typename add_volatile_if<Cond, T>::type;

    namespace detail
    {
        template <typename HashedType, size_t HashSize>
        class xxhash;
    }

#undef  SAFE_STATIC
#if SUPDEF_MULTITHREADED
# define SAFE_STATIC static thread_local
#else
# define SAFE_STATIC static
#endif
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

    namespace detail
    {
        struct copy_cv_from_impl_base
        {
            enum class add
            {
                none, lref, rref
            };

            template <typename T>
            static consteval add get_add()
            {
                if constexpr (std::is_lvalue_reference_v<T>)
                    return add::lref;
                else if constexpr (std::is_rvalue_reference_v<T>)
                    return add::rref;
                else
                    return add::none;
            }
        };

        template <typename FromT, typename ToT>
        struct copy_cv_from_impl
            : public copy_cv_from_impl_base
        {
            using raw_type = add_const_if_t<
                std::is_const_v<FromT>,
                add_volatile_if_t<
                    std::is_volatile_v<FromT>,
                    std::remove_cv_t<ToT>
                >
            >;

            template <add What>
            using type = std::conditional_t<
                What == add::none,
                raw_type,
                std::conditional_t<
                    What == add::lref,
                    std::add_lvalue_reference_t<raw_type>,
                    std::add_rvalue_reference_t<raw_type>
                >
            >;
        };
    }

    template <typename FromT, typename ToT>
    struct copy_cv_from
    {
        using type = typename detail::copy_cv_from_impl<
            std::remove_reference_t<FromT>,
            std::remove_reference_t<ToT>
        >::template type<
            detail::copy_cv_from_impl_base::get_add<ToT>()
        >;
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
        requires (N < sizeof...(Ts)) && (sizeof...(Ts) > 0)
    struct nth_type
    {
        using type = typename detail::nth_type_impl<N, Ts...>::type;
    };

    template <size_t N, typename... Ts>
        requires (N < sizeof...(Ts)) && (sizeof...(Ts) > 0)
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

    static_assert(
        is_type_complete<int>(),
        "`bool is_type_complete<T>()` is broken"
    );

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
    using bigrational_base = boostmp::mpq_rational;
    using bigcomplex_base = boostmp::mpfi_float;

    static_assert(
        std::bool_constant<
            is_type_complete<bigint_base>()
        >::value && std::bool_constant<
            is_type_complete<bigfloat_base>()
        >::value && std::bool_constant<
            is_type_complete<bigrational_base>()
        >::value && std::bool_constant<
            is_type_complete<bigcomplex_base>()
        >::value,
        "missed one boost::multiprecision include"
    );

    struct big_numbers_base
    {
        template <typename T, typename Self>
            requires std::constructible_from<T, Self&&>
        constexpr T as(this Self&& self)
        {
            return T{std::forward<Self>(self)};
        }

        template <typename T, typename Self>
            requires (!std::constructible_from<T, Self&&> &&
                       std::convertible_to<Self&&, T>)
        constexpr T as(this Self&& self)
        {
            return static_cast<T>(std::forward<Self>(self));
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

    struct bigrational
        : public virtual big_numbers_base
        , public bigrational_base
    {
        using bigrational_base::bigrational_base;

        template <typename FwdT1, typename FwdT2>
            requires std::derived_from<std::remove_reference_t<FwdT1>, big_numbers_base> &&
                     std::derived_from<std::remove_reference_t<FwdT2>, big_numbers_base>
        static constexpr bigrational from(FwdT1&& num, FwdT2&& den);
    };

    template <typename FwdT1, typename FwdT2>
        requires std::derived_from<std::remove_reference_t<FwdT1>, big_numbers_base> &&
                 std::derived_from<std::remove_reference_t<FwdT2>, big_numbers_base>
    constexpr bigrational bigrational::from(FwdT1&& num, FwdT2&& den)
    {
        return std::forward<FwdT1>(num).template as<bigrational>() / std::forward<FwdT2>(den).template as<bigrational>();
    }

    struct bigcomplex
        : public virtual big_numbers_base
        , public bigcomplex_base
    {
        using bigcomplex_base::bigcomplex_base;
    };

    struct bigdenom;

    // big numerator
    struct bignum
        : public bigint
    {
        using bigint::bigint;

        template <typename Self, typename T>
            requires std::same_as<std::remove_cvref_t<T>, bigdenom>
        constexpr bigrational operator/(this Self&& self, T&& rhs);
    };

    // big denominator
    struct bigdenom
        : public bigint
    {
        using bigint::bigint;
    };

    template <typename Self, typename T>
        requires std::same_as<std::remove_cvref_t<T>, bigdenom>
    constexpr bigrational bignum::operator/(this Self&& self, T&& rhs)
    {
        return bigrational::from(std::forward<Self>(self), std::forward<T>(rhs));
    }

    template <char... Chars>
    constexpr bigint operator""_bigint()
    {
        constexpr char str[sizeof...(Chars) + 1] = {Chars..., '\0'};
        return bigint(std::string_view{str, sizeof...(Chars)});
    }
    constexpr bigint operator""_bigint(const char* str, size_t len)
    {
        return bigint(std::string_view{str, len});
    }

    template <char... Chars>
    constexpr bigfloat operator""_bigfloat()
    {
        constexpr char str[sizeof...(Chars) + 1] = {Chars..., '\0'};
        return bigfloat(std::string_view{str, sizeof...(Chars)});
    }
    constexpr bigfloat operator""_bigfloat(const char* str, size_t len)
    {
        return bigfloat(std::string_view{str, len});
    }

    template <char... Chars>
    constexpr bigrational operator""_bigratio()
    {
        constexpr char str[sizeof...(Chars) + 1] = {Chars..., '\0'};
        return bigrational(std::string_view{str, sizeof...(Chars)});
    }
    constexpr bigrational operator""_bigratio(const char* str, size_t len)
    {
        return bigrational(std::string_view{str, len});
    }

    template <char... Chars>
    constexpr bigcomplex operator""_bigcomplex()
    {
        constexpr char str[sizeof...(Chars) + 1] = {Chars..., '\0'};
        return bigcomplex(std::string_view{str, sizeof...(Chars)});
    }
    constexpr bigcomplex operator""_bigcomplex(const char* str, size_t len)
    {
        return bigcomplex(std::string_view{str, len});
    }

    template <char... Chars>
    constexpr bignum operator""_bignum()
    {
        constexpr char str[sizeof...(Chars) + 1] = {Chars..., '\0'};
        return bignum(std::string_view{str, sizeof...(Chars)});
    }
    constexpr bignum operator""_bignum(const char* str, size_t len)
    {
        return bignum(std::string_view{str, len});
    }

    template <char... Chars>
    constexpr bigdenom operator""_bigdenom()
    {
        constexpr char str[sizeof...(Chars) + 1] = {Chars..., '\0'};
        return bigdenom(std::string_view{str, sizeof...(Chars)});
    }
    constexpr bigdenom operator""_bigdenom(const char* str, size_t len)
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

    namespace detail
    {
        template <template <typename...> class, typename>
        struct is_template_instance_of_impl
            : public std::false_type
        {
        };

        template <template <typename...> class T, typename... Args>
        struct is_template_instance_of_impl<T, T<Args...>>
            : public std::true_type
        {
        };

    }
    template <template <typename...> class TemplT, typename SpecT>
    struct is_template_instance_of
        : public detail::is_template_instance_of_impl<TemplT, SpecT>
    {
    };

    template <template <typename...> class TemplT, typename SpecT>
    inline constexpr bool is_template_instance_of_v = is_template_instance_of<TemplT, SpecT>::value;

    template <typename SpecT, template <typename...> class TemplT>
    concept specialization_of = is_template_instance_of_v<TemplT, SpecT>;

    /**
     * @brief This is a constexpr-compatible type list that can be used to
     *        store types in a list-like manner. Note that its `value_type`
     *        is a `std::variant` of all the types in the list + an `integral_constant`
     *        of the index of the type in the list (to disambiguate between potentially
     *        identical types in the variant).
     *
     * @tparam Ts The types in the list.
     */
    template <typename... Ts>
    struct typelist
    {
    private:
        using index_seq_type = std::index_sequence_for<Ts...>;

        template <size_t N>
        static constexpr decltype(auto) value_type_at()
        {
            return hana::type_c<
                std::pair<
                    std::integral_constant<size_t, N>,
                    hana::type<typename nth_type<N, Ts...>::type>
                >
            >;
        }
        template <size_t N>
        static constexpr
        typename decltype(value_type_at<N>())::type value_at()
        {
            return { { }, { } };
        }

        template <size_t... Is>
        static constexpr decltype(auto) mk_value_type(std::index_sequence<Is...>)
        {
            return hana::type_c<
                std::variant<
                    typename decltype(value_type_at<Is>())::type...
                >
            >;
        }
        
        template <size_t... Is>
        constexpr typelist(std::index_sequence<Is...>)
            : m_values{ value_at<Is>()... }
        {
        }

    public:
        using value_type = typename decltype(mk_value_type(std::index_sequence_for<Ts...>{}))::type;
        static constexpr size_t size = sizeof...(Ts);

        constexpr typelist()
            : typelist(index_seq_type{})
        {
        }

        constexpr const auto& operator[](size_t idx) const
        {
            return std::get<idx>(m_values);
        }

        constexpr auto begin() const &
        {
            return m_values.begin();
        }

        constexpr auto begin() &&
        {
            return std::make_move_iterator(m_values.begin());
        }

        constexpr auto end() const &
        {
            return m_values.end();
        }

        constexpr auto end() &&
        {
            return std::make_move_iterator(m_values.end());
        }

    private:
        std::array<value_type, sizeof...(Ts)> m_values;
    };

    struct null_iterator
    {
        constexpr null_iterator() = default;

        constexpr null_iterator& operator++()
        {
            return *this;
        }

        constexpr null_iterator operator++(int)
        {
            return *this;
        }

        constexpr bool operator==(const null_iterator&) const
        {
            return true;
        }

        constexpr bool operator!=(const null_iterator&) const
        {
            return false;
        }

        constexpr const std::nullptr_t& operator*() const
        {
            static const std::nullptr_t null = nullptr;
            return null;
        }
    };

    template <>
    struct typelist<>
    {
        using value_type = std::nullptr_t;
        static constexpr size_t size = 0;

        constexpr typelist()
        {
        }

        constexpr const std::nullptr_t& operator[](size_t) const
        {
            static const std::nullptr_t null = nullptr;
            return null;
        }

        constexpr null_iterator begin() const
        {
            return { };
        }

        constexpr null_iterator end() const
        {
            return { };
        }
    };

    namespace detail
    {
        template <typename>
        struct is_typelist_impl
            : public std::false_type
        {
        };

        template <typename... Ts>
        struct is_typelist_impl<typelist<Ts...>>
            : public std::true_type
        {
        };
    }

    template <typename T>
    struct is_typelist
        : public detail::is_typelist_impl<T>
    {
    };

    template <typename T>
    inline constexpr bool is_typelist_v = is_typelist<T>::value;

    namespace detail
    {
        template <size_t N, typename T>
        static constexpr T tuple_of_n_impl_helper()
        {
            return { };
        }

        template <typename T, size_t... Is>
        static constexpr decltype(auto) tuple_of_n_impl(std::index_sequence<Is...>)
        {
            return hana::make_tuple(
                tuple_of_n_impl_helper<Is, T>()...
            );
        }

        template <size_t Count, typename T>
        static constexpr decltype(auto) tuple_of_n()
        {
            return tuple_of_n_impl<T>(std::make_index_sequence<Count>{});
        }

        struct can_convert_to_anything
        {
            template <typename T>
            operator T() const;
        };

        template <typename Fn>
        static consteval decltype(auto) min_arity_of_fn(Fn&&)
        {
            using namespace hana::literals;

            constexpr auto is_invocable_with = hana::is_valid(
                [](auto&&... args)
                    -> decltype(std::declval<Fn&&>()(std::forward<decltype(args)>(args)...))
                { return { }; }
            );

            constexpr auto pred = [is_invocable_with] (auto st) constexpr {
                constexpr auto max_arity = hana::size_c<100>;
                constexpr hana::tuple invoc_args = tuple_of_n<
                    size_t(st), can_convert_to_anything
                >();
                return hana::and_(
                    hana::less.than(max_arity)(st),
                    hana::not_(hana::unpack(invoc_args, is_invocable_with))
                );
            };

            constexpr auto do_inc = [](auto st) constexpr {
                return st + hana::size_c<1>;
            };

            constexpr auto state = hana::size_c<0>;
            constexpr auto ret = hana::while_(pred, state, do_inc);

            return ret;
        }

        template <typename Fn, size_t ICVal>
        static consteval decltype(auto) max_arity_of_fn(hana::size_t<ICVal>, Fn&&)
        {
            constexpr size_t min_arity = ICVal;
            if constexpr (min_arity >= 100)
                return 100;

            using namespace hana::literals;

            constexpr auto is_invocable_with = hana::is_valid(
                [](auto&&... args)
                    -> decltype(std::declval<Fn&&>()(std::forward<decltype(args)>(args)...))
                { return { }; }
            );

            constexpr auto pred = [is_invocable_with] (auto st) constexpr {
                constexpr auto max_arity = hana::size_c<100>;
                constexpr hana::tuple invoc_args = tuple_of_n<
                    size_t(st), can_convert_to_anything
                >();
                return hana::and_(
                    hana::less.than(max_arity)(st),
                    hana::unpack(invoc_args, is_invocable_with)
                );
            };

            constexpr auto do_inc = [](auto st) constexpr {
                return st + hana::size_c<1>;
            };

            constexpr auto state = hana::size_c<min_arity>;
            constexpr auto ret = hana::while_(pred, state, do_inc);

            if constexpr (ret == hana::size_c<100>)
                return hana::size_c<100>;
            else
                return ret - hana::size_c<1>;
        }

        template <size_t ICVal>
        static constexpr decltype(auto) arity_normalize(hana::size_t<ICVal> val)
        {
            constexpr size_t max_size_t = std::numeric_limits<size_t>::max();
            if constexpr (ICVal >= 100)
                return hana::size_c<max_size_t>;
            else
                return val;
        }

        template <typename Fn>
        static constexpr std::pair<size_t, size_t> do_arity_impl(...);

        template <typename Fn>
            requires (
                !requires {
                    typename fn_traits::has_varargs<Fn>;
                }
            )
        static constexpr std::pair<size_t, size_t> do_arity_impl(Fn&& fn)
        {
            constexpr hana::size_t min_arity = decltype(min_arity_of_fn(std::forward<Fn>(fn))){};
            constexpr hana::size_t max_arity = decltype(max_arity_of_fn(min_arity, std::forward<Fn>(fn))){};

            return {
                decltype(min_arity)::value,
                decltype(arity_normalize(max_arity))::value
            };
        }
        

        template <typename Fn>
            requires requires {
                typename fn_traits::has_varargs<Fn>;
            }
        static constexpr std::pair<size_t, size_t> do_arity_impl(Fn&& fn)
        {
            constexpr hana::size_t min_arity = decltype(min_arity_of_fn(std::forward<Fn>(fn))){};
            constexpr hana::size_t max_arity = [&] {
                constexpr size_t max_size_t = std::numeric_limits<size_t>::max();
                if constexpr (!fn_traits::has_varargs_v<Fn>)
                    return decltype(max_arity_of_fn(min_arity, std::forward<Fn>(fn))){};
                else
                    return hana::size_c<max_size_t>;
            }();

            return {
                decltype(min_arity)::value,
                decltype(arity_normalize(max_arity))::value
            };
        }
    }

    struct arity
    {
        std::pair<size_t, size_t> value;
        size_t min;
        size_t max;
        bool is_unbounded;

        template <typename Fn>
        constexpr arity(Fn&& fn) noexcept
            : value(detail::do_arity_impl(std::forward<Fn>(fn)))
            , min(value.first)
            , max(value.second)
            , is_unbounded(max == (size_t)-1)
        {
        }
    };

    template <typename Fn>
    static constexpr arity arity_of(Fn&& fn)
    {
        return arity(std::forward<Fn>(fn));
    }

    /**
     * @brief A container of containers, to ease sequencing of iteration
     *        over multiple containers
     * 
     * @tparam Containers (deduced) The containers to store in the meta container
     * @todo Implement the actual iteration logic
     */
    template <typename... Containers>
    class meta_container
    {
        template <size_t N>
        using container_type_at = std::remove_cvref_t<nth_type_t<N, Containers...>>;

        template <size_t... Is>
        static constexpr decltype(auto) storage_type_impl(std::index_sequence<Is...>)
        {
            return hana::make_tuple(
                std::declval<container_type_at<Is>>()...
            );
        }

        using storage_type = decltype(storage_type_impl(std::index_sequence_for<Containers...>{}));
    
    public:
    private:
        storage_type m_storage;
    };
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
#include <test/arity.hpp>
#include <test/typelist.hpp>

#include <detail/xxhash.hpp>
#include <detail/ckd_arith.hpp>
#include <detail/simple_map.hpp>
#include <detail/globals.hpp>

#endif // TYPES_HPP
