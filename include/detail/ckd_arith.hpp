#ifndef CKD_ARITH_HPP
#define CKD_ARITH_HPP

#include <types.hpp>

#include <version>
#if __cpp_lib_saturation_arithmetic >= 202311L
    #include <numeric>
#endif
#include <utility>
#include <stdexcept>
#include <string>
#include <string_view>
#include <optional>
#include <memory>
#include <algorithm>
#include <functional>
#include <concepts>
#include <type_traits>

#include <stdckdint.h>

#include <cstdbool>

namespace supdef
{
#undef  __CKD_ARITH_DECL
#define __CKD_ARITH_DECL(op)                                        \
    __CKD_ARITH_DECL_NOEXCEPT(op);                                  \
    __CKD_ARITH_DECL_THROWING(op);                                  \
    /**/

#undef  __CKD_ARITH_DECL_NOEXCEPT
#define __CKD_ARITH_DECL_NOEXCEPT(op)                               \
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

#undef  __CKD_ARITH_DECL_THROWING
#define __CKD_ARITH_DECL_THROWING(op)                               \
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

    __CKD_ARITH_DECL(add);
    __CKD_ARITH_DECL(sub);
    __CKD_ARITH_DECL(mul);

#undef __CKD_ARITH_DECL
#undef __CKD_ARITH_DECL_NOEXCEPT
#undef __CKD_ARITH_DECL_THROWING
#undef ckd_add
#undef ckd_sub
#undef ckd_mul

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
