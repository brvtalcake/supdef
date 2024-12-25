#include <iostream>
#include <detail/globals.hpp>
#include <init.hpp>
#include <unicode.hpp>
#include <unicode/regex.h>

#pragma push_macro("CAT")
#pragma push_macro("CATIMPL")
#pragma push_macro("STR")
#pragma push_macro("STRIMPL")

#undef  CAT
#undef  CATIMPL
#define CAT(a, b) CATIMPL(a, b)
#define CATIMPL(a, b) a ## b

#undef  STR
#undef  STRIMPL
#define STR(a) STRIMPL(a)
#define STRIMPL(a) #a

#define _MK_REGEX_GLOBAL(patname_lowercase, patname_uppercase)                  \
    GLOBAL_DEF_START(icu::RegexPattern*, CAT(patname_lowercase, _pattern))      \
        UParseError pe;                                                         \
        UErrorCode status = U_ZERO_ERROR;                                       \
        auto* init = icu::RegexPattern::compile(                                \
            _(CAT(patname_uppercase, _REGEX)), 0, pe, status                    \
        );                                                                      \
        if (U_FAILURE(status))                                                  \
            throw std::runtime_error(                                           \
                "failed to compile " STR(patname_lowercase) " regex"            \
            );                                                                  \
        CAT(patname_lowercase, _pattern) = init;                                \
    GLOBAL_DEF_END(icu::RegexPattern*, CAT(patname_lowercase, _pattern))

#define IMPORT_REGEX R"(^\h*@\h*import\h+\"([^\"]+)\"\h*$)"
#define SUPDEF_BEGIN_REGEX R"(^\h*@\h*supdef((\h+\p{L}+)*)\h+begin\h+([\p{L}_][\p{L}_\p{Nd}]*)\h*$)"
#define SUPDEF_END_REGEX R"(^\h*@\h*end\h*$)"

_MK_REGEX_GLOBAL(import, IMPORT)
_MK_REGEX_GLOBAL(supdef_begin, SUPDEF_BEGIN)
_MK_REGEX_GLOBAL(supdef_end, SUPDEF_END)

#undef _MK_REGEX_GLOBAL

#undef IMPORT_REGEX

#pragma pop_macro("CAT")
#pragma pop_macro("CATIMPL")
#pragma pop_macro("STR")
#pragma pop_macro("STRIMPL")