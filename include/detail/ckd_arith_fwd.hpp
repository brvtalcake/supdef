#ifndef CKD_ARITH_FWD_HPP
#define CKD_ARITH_FWD_HPP

namespace supdef
{
#undef  __CKD_ARITH_DECL
#undef  __CKD_ARITH_DECL_NOEXCEPT
#undef  __CKD_ARITH_DECL_THROWING
#define __CKD_ARITH_DECL(op)                                        \
    __CKD_ARITH_DECL_NOEXCEPT(op);                                  \
    __CKD_ARITH_DECL_THROWING(op);                                  \
    /**/
#define __CKD_ARITH_DECL_NOEXCEPT(op)                               \
    template <std::integral RetT, std::integral T, std::integral U> \
    constexpr bool checked_##op(RetT* ret, T a, U b) noexcept;      \
    template <std::integral RetT, std::integral T, std::integral U> \
    constexpr bool checked_##op(RetT& ret, T a, U b) noexcept       \
    /**/
#define __CKD_ARITH_DECL_THROWING(op)           \
    template <std::integral T, std::integral U> \
    constexpr auto checked_##op(T a, U b)       \
        -> std::common_type_t<T, U>             \
    /**/

    __CKD_ARITH_DECL(add);
    __CKD_ARITH_DECL(sub);
    __CKD_ARITH_DECL(mul);

#undef __CKD_ARITH_DECL
#undef __CKD_ARITH_DECL_NOEXCEPT
#undef __CKD_ARITH_DECL_THROWING

    template <std::integral DestT, std::integral SrcT>
    constexpr bool checked_cast(DestT* ret, SrcT val) noexcept;

    template <std::integral DestT, std::integral SrcT>
    constexpr bool checked_cast(DestT& ret, SrcT val) noexcept;

    template <std::integral DestT, std::integral SrcT>
    constexpr DestT checked_cast(SrcT val);
}

#endif
