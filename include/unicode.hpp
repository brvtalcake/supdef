#ifndef UNICODE_HPP
#define UNICODE_HPP

#include <unicode/unistr.h>
#include <unicode/normalizer2.h>
#include <unicode/utypes.h>
#include <unicode/ucnv.h>
#include <unicode/uchar.h>
#include <unicode/udata.h>

#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <concepts>
#include <cmath>
#include <tgmath.h>

namespace supdef
{
    namespace unicode
    {
        template <typename T>
        static inline T numeric_value(UChar32 c)
        {
            if constexpr (std::floating_point<T>)
                return u_getNumericValue(c);
            else
            {
                auto val = u_getNumericValue(c);
                if (val == U_NO_NUMERIC_VALUE)
                    return 0;
                if constexpr (std::integral<T>)
                    return static_cast<T>(std::llround(val));
                else
                    return static_cast<T>(val);
            }
        }
        template <typename T>
        static inline T numeric_value(const icu::UnicodeString& str, int32_t index)
        {
            return numeric_value<T>(str.char32At(index));
        }
        template <typename T>
        static inline T numeric_value(char32_t c)
        {
            return numeric_value<T>(static_cast<UChar32>(c));
        }
    }
    namespace unicode::normalizer
    {
        static inline const icu::Normalizer2* get_instance(
            const char* norm_name = "nfc",
            UNormalization2Mode mode = UNORM2_COMPOSE
        )
        {
            UErrorCode status = U_ZERO_ERROR;
            const auto* norm = icu::Normalizer2::getInstance(nullptr, norm_name, mode, status);
            if (U_FAILURE(status))
                return nullptr;
            return norm;
        }

        static inline bool is_normalized(
            const icu::UnicodeString& str,
            const icu::Normalizer2* norm
        )
        {
            UErrorCode status = U_ZERO_ERROR;
            if (!norm)
                return false;
            auto result = norm->isNormalized(str, status);
            if (U_FAILURE(status))
                return false;
            return result;
        }

        static inline bool is_normalized(
            const icu::UnicodeString& str,
            const char* norm_name = "nfc",
            UNormalization2Mode mode = UNORM2_COMPOSE
        )
        {
            auto* norm = get_instance(norm_name, mode);
            return is_normalized(str, norm);
        }

        static inline icu::UnicodeString normalize(
            const icu::UnicodeString& str,
            const icu::Normalizer2* norm
        )
        {
            UErrorCode status = U_ZERO_ERROR;
            if (!norm)
                return str;
            int32_t yesend = norm->spanQuickCheckYes(str, status);
            if (U_FAILURE(status) || yesend < 0)
                throw std::runtime_error("failed to normalize string");
            status = U_ZERO_ERROR;
            icu::UnicodeString goodpart = str.tempSubString(0, yesend);
            icu::UnicodeString badpart = str.tempSubString(yesend);
            icu::UnicodeString result = norm->normalizeSecondAndAppend(goodpart, badpart, status);
            if (U_FAILURE(status))
                throw std::runtime_error("failed to normalize string");
            return result;
        }

        static inline icu::UnicodeString normalize(
            const icu::UnicodeString& str,
            const char* norm_name = "nfc",
            UNormalization2Mode mode = UNORM2_COMPOSE
        )
        {
            auto* norm = get_instance(norm_name, mode);
            return normalize(str, norm);
        }
    }

    namespace uninorm = unicode::normalizer;

    namespace unicode::category
    {
        static inline UCharCategory get_char_category(UChar32 c)
        {
            return static_cast<UCharCategory>(u_charType(c));
        }

        static inline UCharCategory get_char_category(const icu::UnicodeString& str, int32_t index)
        {
            return get_char_category(str.char32At(index));
        }

        namespace detail
        {
            using is_ctype_fn = UBool (*)(UChar32);
            static consteval is_ctype_fn select_unicode_fn(std::string&& ctype)
            {
#undef  __UNICODE_FN
#define __UNICODE_FN(pred)  \
    if (ctype == #pred)     \
        return &u_is##pred;

                __UNICODE_FN(alnum)
                __UNICODE_FN(alpha)
                __UNICODE_FN(blank)
                __UNICODE_FN(cntrl)
                __UNICODE_FN(defined)
                __UNICODE_FN(digit)
                __UNICODE_FN(graph)
                __UNICODE_FN(lower)
                __UNICODE_FN(print)
                __UNICODE_FN(punct)
                __UNICODE_FN(space)
                __UNICODE_FN(title)
                __UNICODE_FN(upper)
                __UNICODE_FN(xdigit)

                return nullptr;
            }
        }

#undef  __ENABLEIF_2
#undef  __ENABLEIF_1
#define __ENABLEIF_2(type)          \
    std::enable_if_t<               \
        std::conjunction<           \
            std::negation<          \
                std::is_same<       \
                    UChar32,        \
                    uint32_t        \
                >                   \
            >,                      \
            std::negation<          \
                std::is_same<       \
                    uint32_t,       \
                    uint_least32_t  \
                >                   \
            >                       \
        >::value,                   \
        type                        \
    >
#define __ENABLEIF_1(type)  \
    std::enable_if_t<       \
        std::negation<      \
            std::is_same<   \
                UChar32,    \
                uint32_t    \
            >               \
        >::value,           \
        type                \
    >

#undef  __UNICODE_FN
#define __UNICODE_FN(pred)                              \
    static inline bool is_##pred(UChar32 c)             \
    { return detail::select_unicode_fn(#pred)(c); }     \
    template <typename T = uint_least32_t>              \
    static inline bool is_##pred(                       \
        __ENABLEIF_2(T) c                               \
    ) { return is_##pred(static_cast<UChar32>(c)); }    \
    template <typename T = uint32_t>                    \
    static inline bool is_##pred(                       \
        __ENABLEIF_1(T) c                               \
    ) { return is_##pred(static_cast<UChar32>(c)); }    \
    static inline bool is_##pred(                       \
        char32_t c                                      \
    ) { return is_##pred(static_cast<UChar32>(c)); }    \
    static inline bool is_##pred(                       \
        const icu::UnicodeString& str,                  \
        int32_t index                                   \
    ) { return is_##pred(str.char32At(index)); }

        __UNICODE_FN(alnum)
        __UNICODE_FN(alpha)
        __UNICODE_FN(blank)
        __UNICODE_FN(cntrl)
        __UNICODE_FN(defined)
        __UNICODE_FN(digit)
        __UNICODE_FN(graph)
        __UNICODE_FN(lower)
        __UNICODE_FN(print)
        __UNICODE_FN(punct)
        __UNICODE_FN(space)
        __UNICODE_FN(title)
        __UNICODE_FN(upper)
        __UNICODE_FN(xdigit)

        static inline bool is_odigit(UChar32 c)
        {
            return ::supdef::unicode::category::is_digit(c) &&
                   ::supdef::unicode::numeric_value<uint8_t>(c) < 8;
        }
        template <typename T = uint_least32_t>
        static inline bool is_odigit(__ENABLEIF_2(T) c)
        {
            return is_odigit(static_cast<UChar32>(c));
        }
        template <typename T = uint32_t>
        static inline bool is_odigit(__ENABLEIF_1(T) c)
        {
            return is_odigit(static_cast<UChar32>(c));
        }
        static inline bool is_odigit(char32_t c)
        {
            return is_odigit(static_cast<UChar32>(c));
        }
    }

    namespace unicat = unicode::category;

    namespace unicode::regex
    {
        // TODO
    }
}

#undef  _
#if !defined(UNICODE_STRING_SIMPLE) || 1
    #define _(...) ::supdef::uninorm::normalize(icu::UnicodeString::fromUTF8(__VA_ARGS__))
#else
    #define _(...) UNICODE_STRING_SIMPLE(__VA_ARGS__)
#endif

#undef  FASTSTRING
#define FASTSTRING(str) UNICODE_STRING_SIMPLE(str)

#endif
