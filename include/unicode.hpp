#ifndef UNICODE_HPP
#define UNICODE_HPP

#include <types.hpp>

namespace supdef
{
    template <typename T>
    static inline constexpr bool is_valid_type_v = std::is_void_v<std::void_t<T>>;

    template <typename T>
    static inline constexpr bool has_valid_numlimits_v = is_valid_type_v<std::numeric_limits<T>> &&
                                                         std::numeric_limits<T>::is_specialized;

    namespace unicode
    {
        template <typename T, std::enable_if_t<has_valid_numlimits_v<T>, bool> Strict = true>
        static inline std::optional<T> numeric_value(UChar32 c)
        {
            static_assert(
                std::tuple_size_v<fn_traits::args_t<decltype(u_getNumericValue)>> == 1
            );
            static_assert(
                std::same_as<
                    std::tuple_element_t<0, fn_traits::args_t<decltype(u_getNumericValue)>>,
                    UChar32
                >
            );
            static_assert(
                std::same_as<
                    std::invoke_result_t<decltype(u_getNumericValue), UChar32>,
                    double
                >
            );

            double val = u_getNumericValue(c);
            if (val == U_NO_NUMERIC_VALUE)
                return std::nullopt;

            if constexpr (std::floating_point<T>)
                return static_cast<T>(val);
            else
            {
                if constexpr (Strict)
                {
                    long double lval = val;
                    long double min = (long double)std::numeric_limits<T>::lowest();
                    long double max = (long double)std::numeric_limits<T>::max();
                    if (lval < min || lval > max)
                        return std::nullopt;
                }

                if constexpr (std::integral<T>)
                    return static_cast<T>(std::llround(val));
                else
                    return static_cast<T>(val);
            }
        }
        template <typename T, std::enable_if_t<has_valid_numlimits_v<T>, bool> Strict = true>
        static inline std::optional<T> numeric_value(const icu::UnicodeString& str, int32_t index)
        {
            return numeric_value<T, Strict>(str.char32At(index));
        }
        template <typename T, std::enable_if_t<has_valid_numlimits_v<T>, bool> Strict = true>
        static inline std::optional<T> numeric_value(char32_t c)
        {
            return numeric_value<T, Strict>(static_cast<UChar32>(c));
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
                   ::supdef::unicode::numeric_value<uint8_t, true>(c) < 8;
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

        static inline bool is_bindigit(UChar32 c)
        {
            bool isa_digit = ::supdef::unicode::category::is_digit(c);
            if (!isa_digit)
                return false;
            auto numval = ::supdef::unicode::numeric_value<uint8_t, true>(c);
            return numval < 2;
        }
        template <typename T = uint_least32_t>
        static inline bool is_bindigit(__ENABLEIF_2(T) c)
        {
            return is_bindigit(static_cast<UChar32>(c));
        }
        template <typename T = uint32_t>
        static inline bool is_bindigit(__ENABLEIF_1(T) c)
        {
            return is_bindigit(static_cast<UChar32>(c));
        }
        static inline bool is_bindigit(char32_t c)
        {
            return is_bindigit(static_cast<UChar32>(c));
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

#include <simdutf.h>

namespace supdef
{
    namespace mkliterals
    {
        namespace impl
        {
            static constexpr char32_t operator""_u32_char_(const char* str, size_t length)
            {
                using namespace simdutf;
                size_t required_length = utf32_length_from_utf8(str, length);
                assert(required_length == 1);
                char32_t ret;
                convert_valid_utf8_to_utf32(str, length, &ret);
                return ret;
            }

            static constexpr std::u32string operator""_u32_str_(const char* str, size_t length)
            {
                using namespace simdutf;
                size_t required_length = utf32_length_from_utf8(str, length);
                std::u32string ret(required_length, U'\0');
                convert_valid_utf8_to_utf32(str, length, ret.data());
                return ret;
            }
        }
    }
}

using namespace supdef::mkliterals::impl;

#undef  UPPCONCAT
#undef  UPPCONCAT_IMPL
#define UPPCONCAT(tok1, ...) UPPCONCAT_IMPL(tok1, __VA_ARGS__)
#define UPPCONCAT_IMPL(tok1, ...) tok1##__VA_ARGS__

#undef  UPPSTRINGIZE
#undef  UPPSTRINGIZE_IMPL
#define UPPSTRINGIZE(...) UPPSTRINGIZE_IMPL(__VA_ARGS__)
#define UPPSTRINGIZE_IMPL(...) #__VA_ARGS__

#undef  UCHR
#define UCHR(...) UPPCONCAT(UPPSTRINGIZE(__VA_ARGS__), _u32_char_)

#undef  USTR
#define USTR(...) UPPCONCAT(UPPSTRINGIZE(__VA_ARGS__), _u32_str_)

/*
 * USTR(Hello," " world!)       ---> "Hello,\" \" world!"_u32_str_
 * USTR(Cela coûte environ 10€) ---> "Cela coûte environ 10€"_u32_str_
 * USTR(أنا أحب البيتزا)             ---> "أنا أحب البيتزا"_u32_str_
 */

/*
 * UCHR(ب)  ---> "ب"_u32_char_
 * UCHR(🩎)  ---> "🩎"_u32_char_
 */
#endif
