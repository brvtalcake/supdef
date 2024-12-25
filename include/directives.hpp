#ifndef DIRECTIVES_HPP
#define DIRECTIVES_HPP

#include <types.hpp>
#include <unicode.hpp>

#include <variant>

namespace supdef
{
    enum toplevel_node_kind
    {
        TOPLVL_STRING,   // string
        TOPLVL_IMPORT,   // @import "<file>"
        TOPLVL_SUPDEF,   // @supdef <options> begin <name>
        TOPLVL_EMBED,    // @embed "<file>"
        TOPLVL_DUMP,     // @dump <number>
        TOPLVL_END,      // @end
        TOPLVL_CODE      // other code
    };

    enum supdef_node_kind
    {
        SD_LET,
        SD_IF,
        SD_ELIF,
        SD_ELSE,
        SD_ENDIF,
        SD_FUNCTION,
        SD_CODE
    };

    enum function_node_kind
    {
        FUNC_JOIN,
        FUNC_SPLIT,
        FUNC_STR,
        FUNC_UNSTR,
        FUNC_LEN
    };

    struct toplevel_node
    {
        toplevel_node_kind kind;
        location loc;
        icu::UnicodeString content;
    };

    struct supdef_node
    {
        supdef_node_kind kind;
        location loc;
        icu::UnicodeString content;
    };

    struct function_node
    {
        function_node_kind kind;
        location loc;
        icu::UnicodeString content;
    };

    using node = std::variant<toplevel_node, supdef_node, function_node>;

    class tree
    {
    public:
        tree() = default;
        ~tree() = default;

        void add_node(const node& n) noexcept;
        void operator()() noexcept;
    };
}

#endif
