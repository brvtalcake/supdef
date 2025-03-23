#include <boost/preprocessor/seq/elem.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/for_each_i.hpp>
#include <boost/preprocessor/seq/size.hpp>
#include <boost/preprocessor/seq/enum.hpp>
#include <boost/preprocessor/seq/to_tuple.hpp>
#include <boost/preprocessor/seq/variadic_seq_to_seq.hpp>
#include <boost/preprocessor/variadic/elem.hpp>
#include <boost/preprocessor/tuple/rem.hpp>
#include <boost/preprocessor/comma_if.hpp>
#include <boost/preprocessor/logical/bool.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/iteration.hpp>
#include <boost/preprocessor/iterate.hpp>

#undef  __MK_TEST_STRING_STRINGIZE_IMPL
#define __MK_TEST_STRING_STRINGIZE_IMPL(...) #__VA_ARGS__

#undef  __MK_TEST_STRING_STRINGIZE
#define __MK_TEST_STRING_STRINGIZE(...) __MK_TEST_STRING_STRINGIZE_IMPL(__VA_ARGS__)

#undef  __MK_TEST_STRING
#define __MK_TEST_STRING(instantiation, expected) BOOST_PP_STRINGIZE(instantiation is not expected)

#undef  __DO_REM_FOREACHI
#define __DO_REM_FOREACHI(r, _, i, elem) BOOST_PP_COMMA_IF(BOOST_PP_BOOL(i)) BOOST_PP_TUPLE_REM_CTOR(elem)

#undef  __VSEQ_TO_VARIADIC_IMPL
#define __VSEQ_TO_VARIADIC_IMPL(vseq) BOOST_PP_SEQ_FOR_EACH_I(__DO_REM_FOREACHI, ~,  BOOST_PP_VARIADIC_SEQ_TO_SEQ(vseq))

#undef  __VSEQ_TO_VARIADIC
#define __VSEQ_TO_VARIADIC(vseq) __VSEQ_TO_VARIADIC_IMPL(vseq)

#undef  __MK_TEST_IMPL
#define __MK_TEST_IMPL(num, typevseq, expected)                                         \
    static_assert(                                                                      \
        std::same_as<                                                                   \
            nth_type_t<num, __VSEQ_TO_VARIADIC(typevseq)>,                              \
            BOOST_PP_TUPLE_REM_CTOR(expected)                                           \
        >, __MK_TEST_STRING(                                                            \
            __MK_TEST_STRING_STRINGIZE(nth_type_t<num, __VSEQ_TO_VARIADIC(typevseq)),   \
            __MK_TEST_STRING_STRINGIZE(BOOST_PP_TUPLE_REM_CTOR(expected))               \
        )                                                                               \
    )                                                                                   \
    /**/

#undef  __DO_MK_TEST_IMPL
#define __DO_MK_TEST_IMPL(r, typevseq, i, type) \
    __MK_TEST_IMPL(                             \
        i,                                      \
        typevseq,                               \
        type                                    \
    )                                           \
    /**/

#undef  __MK_TEST
#define __MK_TEST(typevseq)                         \
    BOOST_PP_EXPAND(                                \
        BOOST_PP_SEQ_FOR_EACH_I(                    \
            __DO_MK_TEST_IMPL,                      \
            typevseq,                               \
            BOOST_PP_VARIADIC_SEQ_TO_SEQ(typevseq)  \
        )                                           \
    )                                               \
    /**/

namespace supdef
{
#ifndef __INTELLISENSE__
#undef  __TEST_ARGS
#define __TEST_ARGS (int* const&)
#define BOOST_PP_ITERATION_PARAMS_1 (3, (1, BOOST_PP_SEQ_SIZE(BOOST_PP_VARIADIC_SEQ_TO_SEQ(__TEST_ARGS)), <test/pp-iter/nth_type-test.hpp>))
#include BOOST_PP_ITERATE()

#undef  __TEST_ARGS
#define __TEST_ARGS (int const&)(double&&)(char)
#define BOOST_PP_ITERATION_PARAMS_1 (3, (1, BOOST_PP_SEQ_SIZE(BOOST_PP_VARIADIC_SEQ_TO_SEQ(__TEST_ARGS)), <test/pp-iter/nth_type-test.hpp>))
#include BOOST_PP_ITERATE()

#undef  __TEST_ARGS
#define __TEST_ARGS (int)(double)(char)(std::unique_ptr<int, std::default_delete<int>>)
#define BOOST_PP_ITERATION_PARAMS_1 (3, (1, BOOST_PP_SEQ_SIZE(BOOST_PP_VARIADIC_SEQ_TO_SEQ(__TEST_ARGS)), <test/pp-iter/nth_type-test.hpp>))
#include BOOST_PP_ITERATE()
#else
    #error "do not define __INTELLISENSE__ when compiling"
    /* previous lines expand to: */
    static_assert(
        std::same_as< nth_type_t<0, int* const& >, int* const& >, "\"nth_type_t<0, int* const&\" is not \"int* const&\""
    );
    static_assert(
        std::same_as< nth_type_t<0, int const& , double&&, char >, int const& >, "\"nth_type_t<0, int const& , double&&, char\" is not \"int const&\""
    );
    static_assert(
        std::same_as< nth_type_t<1, int const& , double&&, char >, double&& >, "\"nth_type_t<1, int const& , double&&, char\" is not \"double&&\""
    );
    static_assert(
        std::same_as< nth_type_t<2, int const& , double&&, char >, char >, "\"nth_type_t<2, int const& , double&&, char\" is not \"char\""
    );
    static_assert(
        std::same_as< nth_type_t<0, int , double , char , std::unique_ptr<int, std::default_delete<int>> >, int >, "\"nth_type_t<0, int , double , char , std::unique_ptr<int, std::default_delete<int>>\" is not \"int\""
    );
    static_assert(
        std::same_as< nth_type_t<1, int , double , char , std::unique_ptr<int, std::default_delete<int>> >, double >, "\"nth_type_t<1, int , double , char , std::unique_ptr<int, std::default_delete<int>>\" is not \"double\""
    );
    static_assert(
        std::same_as< nth_type_t<2, int , double , char , std::unique_ptr<int, std::default_delete<int>> >, char >, "\"nth_type_t<2, int , double , char , std::unique_ptr<int, std::default_delete<int>>\" is not \"char\""
    );
    static_assert(
        std::same_as< nth_type_t<3, int , double , char , std::unique_ptr<int, std::default_delete<int>> >, std::unique_ptr<int, std::default_delete<int>> >, "\"nth_type_t<3, int , double , char , std::unique_ptr<int, std::default_delete<int>>\" is not \"std::unique_ptr<int, std::default_delete<int>>\""
    );
#endif

}

#undef __DO_MK_TEST_IMPL
#undef __DO_REM_FOREACHI
#undef __MK_TEST
#undef __MK_TEST_IMPL
#undef __VSEQ_TO_VARIADIC
#undef __VSEQ_TO_VARIADIC_IMPL
#undef __MK_TEST_STRING
#undef __MK_TEST_STRING_STRINGIZE
#undef __MK_TEST_STRING_STRINGIZE_IMPL