#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include <init.hpp>
#include <types.hpp>

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/control/if.hpp>
#include <boost/preprocessor/facilities/is_empty_variadic.hpp>
#include <boost/preprocessor/tuple/rem.hpp>

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
            extern type& GLOBAL_CAT(get_, name) ()  \
                noexcept;                           \
        }                                           \
    }

#undef  GLOBAL_TEMPLATED_XXHASHER_DECL
#define GLOBAL_TEMPLATED_XXHASHER_DECL(type, name, ...) \
    namespace supdef {                                  \
        namespace globals {                             \
            template <>                                 \
            type& GLOBAL_CAT(get_, name)                \
                __VA_ARGS__ ()                          \
                noexcept;                               \
        }                                               \
    }

#undef  GLOBAL_XXHASHER_DECL
#define GLOBAL_XXHASHER_DECL(type, hashsize, ...)       \
    BOOST_PP_TUPLE_REM_CTOR(                            \
        BOOST_PP_IF(                                    \
            BOOST_PP_IS_EMPTY(__VA_ARGS__),             \
            (GLOBAL_GETTER_DECL(                        \
                BOOST_IDENTITY_TYPE((                   \
                    ::supdef::detail::xxhash<           \
                        type, hashsize                  \
                    >                                   \
                )),                                     \
                BOOST_PP_CAT(                           \
                    type, BOOST_PP_CAT(                 \
                        _xxhasher, hashsize             \
                    )                                   \
                )                                       \
            )),                                         \
            (GLOBAL_GETTER_DECL(                        \
                BOOST_IDENTITY_TYPE((                   \
                    ::supdef::detail::xxhash<           \
                        type, hashsize                  \
                    >                                   \
                )),                                     \
                BOOST_PP_CAT(                           \
                    __VA_ARGS__, BOOST_PP_CAT(          \
                        _xxhasher, hashsize             \
                    )                                   \
                )                                       \
            ))                                          \
        )                                               \
    )                                                   \
    GLOBAL_TEMPLATED_XXHASHER_DECL(                     \
        BOOST_IDENTITY_TYPE((                           \
            ::supdef::detail::xxhash<                   \
                type, hashsize                          \
            >                                           \
        )),                                             \
        xxhasher, <type, hashsize>                      \
    )

#undef  __XXHASHER_VARNAME
#define __XXHASHER_VARNAME(type, hashsize, ...)         \
    BOOST_PP_IF(                                        \
        BOOST_PP_IS_EMPTY(__VA_ARGS__),                 \
        BOOST_PP_CAT(                                   \
            type, BOOST_PP_CAT(                         \
                _xxhasher, hashsize                     \
            )                                           \
        ),                                              \
        BOOST_PP_CAT(                                   \
            __VA_ARGS__, BOOST_PP_CAT(                  \
                _xxhasher, hashsize                     \
            )                                           \
        )                                               \
    )

#undef  GLOBAL_XXHASHER_DEF
#if 0
#define GLOBAL_XXHASHER_DEF(type, hashsize, ...)            \
    BOOST_PP_TUPLE_REM_CTOR(                                \
        BOOST_PP_IF(                                        \
            BOOST_PP_IS_EMPTY(__VA_ARGS__),                 \
            (GLOBAL_DEF_START(                              \
                BOOST_IDENTITY_TYPE((                       \
                    ::supdef::detail::xxhash<               \
                        type, hashsize                      \
                    >                                       \
                )),                                         \
                BOOST_PP_CAT(                               \
                    type, BOOST_PP_CAT(_xxhasher, hashsize) \
                )                                           \
            )),                                             \
            (GLOBAL_DEF_START(                              \
                BOOST_IDENTITY_TYPE((                       \
                    ::supdef::detail::xxhash<               \
                        type, hashsize                      \
                    >                                       \
                )),                                         \
                BOOST_PP_CAT(                               \
                    __VA_ARGS__, BOOST_PP_CAT(              \
                        _xxhasher, hashsize                 \
                    )                                       \
                )                                           \
            ))                                              \
        )                                                   \
    )                                                       \
        return ::supdef::detail::xxhash<type, hashsize>(    \
            seed_with_time()                                \
        );                                                  \
    BOOST_PP_TUPLE_REM_CTOR(                                \
        BOOST_PP_IF(                                        \
            BOOST_PP_IS_EMPTY(__VA_ARGS__),                 \
            (GLOBAL_DEF_END(                                \
                BOOST_IDENTITY_TYPE((                       \
                    ::supdef::detail::xxhash<               \
                        type, hashsize                      \
                    >                                       \
                )),                                         \
                BOOST_PP_CAT(                               \
                    type, BOOST_PP_CAT(_xxhasher, hashsize) \
                )                                           \
            )),                                             \
            (GLOBAL_DEF_END(                                \
                BOOST_IDENTITY_TYPE((                       \
                    ::supdef::detail::xxhash<               \
                        type, hashsize                      \
                    >                                       \
                )),                                         \
                BOOST_PP_CAT(                               \
                    __VA_ARGS__, BOOST_PP_CAT(              \
                        _xxhasher, hashsize                 \
                    )                                       \
                )                                           \
            ))                                              \
        )                                                   \
    )                                                       \
    namespace supdef {                                      \
        namespace globals {                                 \
            template <>                                     \
            ::supdef::detail::xxhash<type, hashsize>&       \
            get_xxhasher<type, hashsize> ()                 \
                noexcept                                    \
            {                                               \
                return BOOST_PP_IF(                         \
                    BOOST_PP_IS_EMPTY(__VA_ARGS__),         \
                    (BOOST_PP_CAT(                          \
                        get_,                               \
                        BOOST_PP_CAT(                       \
                            type, BOOST_PP_CAT(             \
                                _xxhasher, hashsize         \
                            )                               \
                        )                                   \
                    )),                                     \
                    (BOOST_PP_CAT(                          \
                        get_, BOOST_PP_CAT(                 \
                            __VA_ARGS__, BOOST_PP_CAT(      \
                                _xxhasher, hashsize         \
                            )                               \
                        )                                   \
                    ))                                      \
                )();                                        \
            }                                               \
        }                                                   \
    }
#else
#define GLOBAL_XXHASHER_DEF(type, hashsize, ...)            \
    GLOBAL_DEF_START(                                       \
        BOOST_IDENTITY_TYPE((                               \
            ::supdef::detail::xxhash<                       \
                type, hashsize                              \
            >                                               \
        )),                                                 \
        __XXHASHER_VARNAME(type, hashsize, __VA_ARGS__)     \
    )                                                       \
        __XXHASHER_VARNAME(type, hashsize, __VA_ARGS__) =   \
            ::supdef::detail::xxhash<type, hashsize>(       \
                seed_with_time()                            \
            );                                              \
    GLOBAL_DEF_END(                                         \
        BOOST_IDENTITY_TYPE((                               \
            ::supdef::detail::xxhash<                       \
                type, hashsize                              \
            >                                               \
        )),                                                 \
        __XXHASHER_VARNAME(type, hashsize, __VA_ARGS__)     \
    )                                                       \
    namespace supdef {                                      \
        namespace globals {                                 \
            template <>                                     \
            ::supdef::detail::xxhash<type, hashsize>&       \
            get_xxhasher<type, hashsize> ()                 \
                noexcept                                    \
            {                                               \
                return BOOST_PP_CAT(                        \
                    get_, __XXHASHER_VARNAME(               \
                        type, hashsize, __VA_ARGS__         \
                    )                                       \
                )();                                        \
            }                                               \
        }                                                   \
    }
#endif
//GLOBAL_XXHASHER_DECL(BOOST_IDENTITY_TYPE((std::vector<std::string, std::pmr::polymorphic_allocator<std::string>>)), 64, vector_string)
//GLOBAL_XXHASHER_DEF (BOOST_IDENTITY_TYPE((std::vector<std::string, std::pmr::polymorphic_allocator<std::string>>)), 64, vector_string)


namespace supdef
{
    namespace globals
    {
        template <typename T, size_t S = 64>
        ::supdef::detail::xxhash<T, S>& get_xxhasher() noexcept = delete;
    }
}

#endif
