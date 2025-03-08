#include <version>

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

#ifndef SUPDEF_MAX_PARSING_PHASE
    #define SUPDEF_MAX_PARSING_PHASE 6U
#endif

#ifndef SUPDEF_MULTITHREADED
    #define SUPDEF_MULTITHREADED 0
#endif

#if SUPDEF_MULTITHREADED
    #include <mutex>
    #include <shared_mutex>
    #include <condition_variable>
    #include <atomic>
    #include <thread>
    #include <future>
    #include <barrier>
    #include <semaphore>
    #include <latch>
    #include <stop_token>
    #include <jthread>
    #include <syncstream>
#endif