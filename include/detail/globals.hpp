#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include <init.hpp>

#undef  GLOBAL_CAT_IMPL
#undef  GLOBAL_CAT
#define GLOBAL_CAT_IMPL(a, b) a ## b
#define GLOBAL_CAT(a, b) GLOBAL_CAT_IMPL(a, b)

#undef  GLOBAL_DEF_START
#undef  GLOBAL_DEF_END
#define GLOBAL_DEF_START(type, name)    \
    namespace supdef::globals           \
    {                                   \
        static type name;               \
        INIT_CLIENT_CLASS_PRIO          \
        static supdef::init::client     \
        GLOBAL_CAT(                     \
            ____, GLOBAL_CAT(           \
                name, _initializer____  \
            )                           \
        ) {                             \
            []() static noexcept {

#define GLOBAL_DEF_END(type, name)              \
            }                                   \
        };                                      \
        extern type& GLOBAL_CAT(get_, name)()   \
            noexcept                            \
        {                                       \
            return name;                        \
        }                                       \
    }

#undef  GLOBAL_GETTER_DECL
#define GLOBAL_GETTER_DECL(type, name)              \
    namespace supdef {                              \
        namespace globals {                         \
            extern type& GLOBAL_CAT(get_, name)()   \
                noexcept;                           \
        }                                           \
    }

#endif
