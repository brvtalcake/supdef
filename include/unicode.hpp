#ifndef UNICODE_HPP
#define UNICODE_HPP

#include <unicode/unistr.h>
#include <unicode/normalizer2.h>
#include <unicode/utypes.h>
#include <unicode/ucnv.h>

namespace supdef
{
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

#undef  __UNICODE_FN
#define __UNICODE_FN(pred)                          \
    static inline bool is_##pred(UChar32 c)         \
    { return detail::select_unicode_fn(#pred)(c); } \
    static inline bool is_##pred(                   \
        const icu::UnicodeString& str,              \
        int32_t index                               \
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
    }

    namespace unicat = unicode::category;

    namespace unicode::regex
    {
        // TODO
    }
}

#undef  _
#define _(...) ::supdef::uninorm::normalize(icu::UnicodeString::fromUTF8(__VA_ARGS__))

#endif
