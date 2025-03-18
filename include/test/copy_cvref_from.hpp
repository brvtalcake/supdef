#include <boost/preprocessor/seq/elem.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <type_traits>
#include <concepts>

#undef  __MK_TEST_STRING_STRINGIZE_IMPL
#define __MK_TEST_STRING_STRINGIZE_IMPL(...) #__VA_ARGS__

#undef  __MK_TEST_STRING_STRINGIZE
#define __MK_TEST_STRING_STRINGIZE(...) __MK_TEST_STRING_STRINGIZE_IMPL(__VA_ARGS__)

#undef  __MK_TEST_STRING
#define __MK_TEST_STRING(instantiation, expected) BOOST_PP_STRINGIZE(instantiation is not expected)

#undef  __MK_TEST_IMPL
#define __MK_TEST_IMPL(template, from_type, to_type, expected)          \
    static_assert(                                                      \
        std::same_as<                                                   \
            template<from_type, to_type>,                               \
            expected                                                    \
        >, __MK_TEST_STRING(                                            \
            __MK_TEST_STRING_STRINGIZE(template<from_type, to_type>),   \
            __MK_TEST_STRING_STRINGIZE(expected)                        \
        )                                                               \
    )                                                                   \
    /**/
#undef  __MK_TEST
#define __MK_TEST(from_type, to_type, expecteds)    \
    __MK_TEST_IMPL(                                 \
        copy_refs_from_t,                           \
        from_type,                                  \
        to_type,                                    \
        BOOST_PP_SEQ_ELEM(0, expecteds)             \
    );                                              \
    __MK_TEST_IMPL(                                 \
        copy_cv_from_t,                             \
        from_type,                                  \
        to_type,                                    \
        BOOST_PP_SEQ_ELEM(1, expecteds)             \
    );                                              \
    __MK_TEST_IMPL(                                 \
        copy_quals_from_t,                          \
        from_type,                                  \
        to_type,                                    \
        BOOST_PP_SEQ_ELEM(2, expecteds)             \
    )                                               \
    /**/

namespace supdef
{
#ifndef __INTELLISENSE__
    __MK_TEST(
        int, const volatile double&,
        (const volatile double)(double&)(double)
    );
    __MK_TEST(
        int, double&&,
        (double)(double&&)(double)
    );
    __MK_TEST(
        const int&, volatile double,
        (volatile double&)(const double)(const double&)
    );
    __MK_TEST(
        volatile int* const&, double*&&,
        (double*&)(double* const &&)(double* const&)
    );
    __MK_TEST(
        const volatile int* const volatile&, double* const volatile&&,
        (double* const volatile&)(double* const volatile&&)(double* const volatile&)
    );
#else
    #error "do not define __INTELLISENSE__ when compiling"
    static_assert(
        std::same_as<
            copy_refs_from_t<
                int, const volatile double &
            >, const volatile double
        >, "\"copy_refs_from_t<int, const volatile double&>\" is not \"const volatile double\""
    );
    static_assert(
        std::same_as<
            copy_cv_from_t<
                int, const volatile double &
            >, double &
        >, "\"copy_cv_from_t<int, const volatile double&>\" is not \"double&\""
    );
    static_assert(
        std::same_as<
            copy_quals_from_t<
                int, const volatile double &
            >, double
        >, "\"copy_quals_from_t<int, const volatile double&>\" is not \"double\""
    );
    static_assert(
        std::same_as<
            copy_refs_from_t<
                int, double &&
            >, double
        >, "\"copy_refs_from_t<int, double&&>\" is not \"double\""
    );
    static_assert(
        std::same_as<
            copy_cv_from_t<
                int, double &&
            >, double &&
        >, "\"copy_cv_from_t<int, double&&>\" is not \"double&&\""
    );
    static_assert(
        std::same_as<
            copy_quals_from_t<
                int, double &&
            >, double
        >, "\"copy_quals_from_t<int, double&&>\" is not \"double\""
    );
    static_assert(
        std::same_as<
            copy_refs_from_t<
                const int &, volatile double
            >, volatile double &
        >, "\"copy_refs_from_t<const int&, volatile double>\" is not \"volatile double&\""
    );
    static_assert(
        std::same_as<
            copy_cv_from_t<
                const int &, volatile double
            >, const double
        >, "\"copy_cv_from_t<const int&, volatile double>\" is not \"const double\""
    );
    static_assert(
        std::same_as<
            copy_quals_from_t<
                const int &, volatile double
            >, const double &
        >, "\"copy_quals_from_t<const int&, volatile double>\" is not \"const double&\""
    );
    static_assert(
        std::same_as<
            copy_refs_from_t<
                volatile int *const &, double *&&
            >, double *&
        >, "\"copy_refs_from_t<volatile int* const&, double*&&>\" is not \"double*&\""
    );
    static_assert(
        std::same_as<
            copy_cv_from_t<
                volatile int *const &, double *&&
            >, double *const &&
        >, "\"copy_cv_from_t<volatile int* const&, double*&&>\" is not \"double* const &&\""
    );
    static_assert(
        std::same_as<
            copy_quals_from_t<
                volatile int *const &, double *&&
            >, double *const &
        >, "\"copy_quals_from_t<volatile int* const&, double*&&>\" is not \"double* const&\""
    );
    static_assert(
        std::same_as<
            copy_refs_from_t<
                const volatile int *const volatile &, double *const volatile &&
            >, double *const volatile &
        >, "\"copy_refs_from_t<const volatile int* const volatile&, double* const volatile&&>\" is not \"double* const volatile&\""
    );
    static_assert(
        std::same_as<
            copy_cv_from_t<
                const volatile int *const volatile &, double *const volatile &&
            >, double *const volatile &&
        >, "\"copy_cv_from_t<const volatile int* const volatile&, double* const volatile&&>\" is not \"double* const volatile&&\""
    );
    static_assert(
        std::same_as<
            copy_quals_from_t<
                const volatile int *const volatile &, double *const volatile &&
            >, double *const volatile &
        >, "\"copy_quals_from_t<const volatile int* const volatile&, double* const volatile&&>\" is not \"double* const volatile&\""
    );
#endif
}

#undef __MK_TEST
#undef __MK_TEST_IMPL
#undef __MK_TEST_STRING
#undef __MK_TEST_STRING_STRINGIZE
#undef __MK_TEST_STRING_STRINGIZE_IMPL
