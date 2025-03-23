#ifndef CKD_ARITH_HPP
#define CKD_ARITH_HPP

#include <types.hpp>
#include "ckd_arith_fwd.hpp"

#include <stdckdint.h>
#include <cstdbool>

namespace supdef
{
#undef  __CKD_ARITH_DEF
#define __CKD_ARITH_DEF(op)                                        \
    __CKD_ARITH_DEF_NOEXCEPT(op);                                  \
    __CKD_ARITH_DEF_THROWING(op);                                  \
    /**/

#undef  __CKD_ARITH_DEF_NOEXCEPT
#define __CKD_ARITH_DEF_NOEXCEPT(op)                                \
    template <std::integral RetT, std::integral T, std::integral U> \
    constexpr bool checked_##op(RetT* ret, T a, U b) noexcept       \
    {                                                               \
        return ckd_##op(ret, a, b);                                 \
    }                                                               \
    template <std::integral RetT, std::integral T, std::integral U> \
    constexpr bool checked_##op(RetT& ret, T a, U b) noexcept       \
    {                                                               \
        return ckd_##op(std::addressof(ret), a, b);                 \
    }                                                               \
    /**/

#undef  __CKD_ARITH_DEF_THROWING
#define __CKD_ARITH_DEF_THROWING(op)                                \
    template <std::integral T, std::integral U>                     \
    constexpr auto checked_##op(T a, U b)                           \
        -> std::common_type_t<T, U>                                 \
    {                                                               \
        using namespace std::string_literals;                       \
        std::common_type_t<T, U> ret;                               \
        if (!checked_##op(std::addressof(ret), a, b))               \
            throw std::overflow_error(                              \
                "overflow in checked_"s + #op + "("s +              \
                    std::to_string(a) + ", "s + std::to_string(b) + \
                ") ("s + __PRETTY_FUNCTION__ + ")"s                 \
            );                                                      \
        return ret;                                                 \
    }                                                               \
    /**/

    __CKD_ARITH_DEF(add);
    __CKD_ARITH_DEF(sub);
    __CKD_ARITH_DEF(mul);

#undef __CKD_ARITH_DEF
#undef __CKD_ARITH_DEF_NOEXCEPT
#undef __CKD_ARITH_DEF_THROWING

    namespace detail
    {
        constexpr bool test_ckd_arith()
        {
            TODO(implement tests);
            return true;
        }
    }

    static_assert(detail::test_ckd_arith());

    template <std::integral DestT, std::integral SrcT>
    constexpr bool checked_cast(DestT* ret, SrcT val) noexcept
    {
        *ret = static_cast<DestT>(val);
        return std::in_range<DestT>(val);
    }

    template <std::integral DestT, std::integral SrcT>
    constexpr bool checked_cast(DestT& ret, SrcT val) noexcept
    {
        return checked_cast(std::addressof(ret), val);
    }

    template <std::integral DestT, std::integral SrcT>
    constexpr DestT checked_cast(SrcT val)
    {
        using namespace std::string_literals;
        DestT ret;
        if (!checked_cast(std::addressof(ret), val))
            throw std::overflow_error(
                "overflow in checked_cast("s +
                    std::to_string(val) +
                ") ("s + __PRETTY_FUNCTION__ + ")"s
            );
        return ret;
    }

    /*
    namespace detail
    {
        // lvalue
        template <typename, typename T, typename StripedT = std::remove_reference_t<T>>
        constexpr T&& forward_to_if_noexcept(StripedT& val) noexcept
        {
            static_assert(std::is_lvalue_reference_v<T>);
            static_assert(std::is_lvalue_reference_v<T&&>);
            return static_cast<T&&>(val);
        }

        // rvalue
        template <typename ToT, typename T, typename StripedT = std::remove_reference_t<T>>
        constexpr decltype(auto) forward_to_if_noexcept(StripedT&& val) noexcept
        {
            static_assert(std::is_rvalue_reference_v<T>);
            static_assert(std::is_rvalue_reference_v<T&&>);

            using cond_t = std::conjunction<
                std::negation<
                    std::is_nothrow_constructible<ToT, StripedT&&>
                >, std::is_constructible<ToT, StripedT&>
            >;
            using cast_t = std::conditional_t<cond_t::value, StripedT&, StripedT&&>;

            return static_cast<cast_t>(val);
        }
    }

    template <std::integral NumT>
    class checked_int
    {
    public:
        using value_type = NumT;

        constexpr checked_int() noexcept = default;

        template <std::convertible_to<NumT> T>
        constexpr checked_int(T&& val) noexcept(std::is_nothrow_constructible_v<NumT, T>)
            : m_val(detail::forward_to_if_noexcept<NumT, T>(val))
        {
        }
    };
    */

    namespace detail
    {
        constexpr bool test_checked_cast()
        {
            TODO(implement tests);
            return true;
        }
    }

    static_assert(detail::test_checked_cast());
}

#endif
