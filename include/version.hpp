#undef  SUPDEF_VERSION_MAJOR
#define SUPDEF_VERSION_MAJOR 0U

#undef  SUPDEF_VERSION_MINOR
#define SUPDEF_VERSION_MINOR 1U

#undef  SUPDEF_VERSION_PATCH
#define SUPDEF_VERSION_PATCH 0U

#ifndef SUPDEF_VERSION
    #define SUPDEF_VERSION (SUPDEF_VERSION_MAJOR * 10000U + SUPDEF_VERSION_MINOR * 100U + SUPDEF_VERSION_PATCH)
#endif

#ifndef SUPDEF_VERSION_STRING
    #define SUPDEF_VERSION_STRING   \
        std::string("v")          + \
        std::to_string(             \
            SUPDEF_VERSION_MAJOR    \
        )                         + \
        "."                       + \
        std::to_string(             \
            SUPDEF_VERSION_MINOR    \
        )                         + \
        "."                       + \
        std::to_string(             \
            SUPDEF_VERSION_PATCH    \
        )
#endif