@import "supdef.sd"

@supdef begin test
    @let to_define = @join($1, _LEN)
    #if !defined($to_define)
        #define $to_define @len($2)
    #endif
@end

// These two lines are equivalent
test(HELLO1, hello world)
test(HELLO2,    hello world        )

/* // "classic" supdef
#pragma supdef begin endifsupdef
"#endif"
#pragma supdef end

#pragma supdef begin test
"#if !defined($1)" 
    "#define $1 $2"
endifsupdef()
#pragma supdef end

test(HELLO, "hello world")
// Expands to:
// #if !defined(HELLO)
//     #define HELLO "hello world"
// #endif

// C runnable supdef (unimplemented)
#pragma supdef runnable C begin test2
    SUPDEF_RETURN($1);
#pragma supdef end

// includes
#pragma supdef import "supdefinitions.sd"

#pragma supdef import "supdefinitions2.sd"

// Dump some code passed to `supdef`
#pragma supdef dump 0

// conditionals (unimplemented)
#pragma supdef if test2(0) != 0
    #include <when_error.h>
#pragma supdef else
    #include <when_ok.h>
#pragma supdef end
// (Expand to `#include <when_ok.h>`)

test2(1) // (Expand to `1`)

// C++ runnable supdef (unimplemented)
#pragma supdef runnable CXX begin test3
SUPDEF_INCLUDE(<string>)
SUPDEF_INCLUDE("/my/cool/header/with/absolute/path.hpp")
std::string returned_str = "constant string";
SUPDEF_RETURN(returned_str + " returned from a runnable");
#pragma supdef end

#warning test3()
// (Expand to `#warning "constant string returned from a runnable"`)

#pragma supdef embed "./data/to_embed.bin"
// Eventually add possibility to pass options (as with standard C "#embed"
 */