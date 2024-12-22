#define BOOST_SPIRIT_UNICODE 1

#include <types.hpp>
#include <directives.hpp>
#include <file.hpp>

// libstdc++
#include <string>
#include <string_view>

#include <unicode/ustream.h>

// libgrapheme
extern "C" {
#include <grapheme.h>
}

#if 0
#include <boost/spirit/home/qi.hpp>
#include <boost/spirit/home/qi/parse.hpp>
#include <boost/spirit/home/qi/numeric.hpp>
#include <boost/spirit/home/qi/char.hpp>
#include <boost/spirit/home/qi/operator.hpp>
#include <boost/spirit/home/qi/string.hpp>
#include <boost/spirit/home/qi/directive.hpp>
#include <boost/spirit/home/qi/nonterminal.hpp>
#include <boost/spirit/home/qi/auxiliary.hpp>

#else
#include <unicode/unistr.h>
#include <unicode/regex.h>

#include <detail/globals.hpp>

GLOBAL_GETTER_DECL(icu::RegexPattern*, import_pattern)

bool matches_import(const icu::UnicodeString& str)
{
    UErrorCode status = U_ZERO_ERROR;
    auto* matcher = ::supdef::globals::get_import_pattern()->matcher(str, status);
    if (U_FAILURE(status))
        throw std::runtime_error("failed to create matcher");
    return matcher->matches(status);
}


#endif