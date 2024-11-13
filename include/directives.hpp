#ifndef DIRECTIVES_HPP
#define DIRECTIVES_HPP

#include <types.hpp>

#include <variant>

namespace supdef
{
    enum toplevel_token_kind
    {
        TOPLVL_IMPORT,
        TOPLVL_SUPDEF,
        TOPLVL_RUNNABLE,
        TOPLVL_EMBED,
        TOPLVL_DUMP,
        TOPLVL_END,
        TOPLVL_CODE
    };

    enum supdef_token_kind
    {
        SD_LET,
        SD_IF,
        SD_ELIF,
        SD_ELSE,
        SD_ENDIF,
        SD_FUNCTION,
        SD_CODE
    };

    enum function_token_kind
    {
        FUNC_JOIN,
        FUNC_STR,
        FUNC_UNSTR,
        FUNC_LEN
    };

    struct toplevel_token
    {
        toplevel_token_kind kind;
        location loc;
        const char* start;
        const char* end;
    };

    struct supdef_token
    {
        supdef_token_kind kind;
        location loc;
        const char* start;
        const char* end;
    };

    struct function_token
    {
        function_token_kind kind;
        location loc;
        const char* start;
        const char* end;
    };

    using token = std::variant<toplevel_token, supdef_token, function_token>;
}

#endif
