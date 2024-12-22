#include <iostream>
#include <detail/globals.hpp>
#include <init.hpp>
#include <unicode/regex.h>

INIT_CLIENT_CLASS_PRIO
static supdef::init::client parser_init_test{
    []() static noexcept {
        /* std::cout << "parser_inits\n"; */
    }
};

#define IMPORT_REGEX R"(^\s*@\s*import\s+\"([^\"]+)\"\s*$)"
/*

#include <detail/globals.hpp>

#define IMPORT_REGEX R"(^\s*@\s*import\s+\"([^\"]+)\"\s*$)"

static icu::RegexPattern import_pattern(IMPORT_REGEX, 0, U_REGEX_CASE_INSENSITIVE, nullptr);
*/

GLOBAL_DEF_START(icu::RegexPattern*, import_pattern)
    UParseError pe;
    UErrorCode status = U_ZERO_ERROR;
    auto* init = icu::RegexPattern::compile(IMPORT_REGEX, 0, pe, status);
    if (U_FAILURE(status))
        throw std::runtime_error("failed to compile import regex");
    import_pattern = init;
GLOBAL_DEF_END(icu::RegexPattern*, import_pattern)