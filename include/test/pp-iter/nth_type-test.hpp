
#include <boost/preprocessor/arithmetic/dec.hpp>

#if BOOST_PP_IS_ITERATING && defined(__TEST_ARGS)

#define i BOOST_PP_DEC(BOOST_PP_ITERATION())
__MK_TEST_IMPL(i, __TEST_ARGS, BOOST_PP_SEQ_ELEM(i, BOOST_PP_VARIADIC_SEQ_TO_SEQ(__TEST_ARGS)));
#undef i

#else
    #error "This file's purpose is to be iterated to generate tests"
#endif
