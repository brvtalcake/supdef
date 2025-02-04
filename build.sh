#!/bin/bash

set -e

files="$(find -L src -name '*.cpp' | xargs)"

function is_defined()
{
    if [ -z ${!1+x} ]; then
        return 1
    else
        return 0
    fi
}

# only define if not already defined
function define()
{
    local name="$1"
    if ! is_defined "$name"; then
        shift
        eval "$name=\"$*\""
    fi
}

function expand_libs()
{
    local _what="--$1"
    for lib in $LIBS; do
        printf ' %s ' "$(pkg-config $_what $lib)"
    done
}

function cmd()
{
    echo "$@"
    eval "$@"
}

define LIBS "icu-io" "libgrapheme" "simdutf" "gmp" "mpfr"
define CPPFLAGS "-Iinclude -D_GNU_SOURCE=1 -DSTATIC_INITIALIZER_ALLOCATION=1"
if [ -z "$OPTIMIZE" ] || [ "$OPTIMIZE" -eq 0 ]; then
    define CFLAGS "$(expand_libs cflags) -pipe -std=gnu++23 -Wall -Wextra -Og -ggdb3 -march=native -mtune=native -flto"
else
    define CFLAGS "$(expand_libs cflags) -pipe -std=gnu++23 -Wall -Wextra -O3 -march=native -mtune=native -flto"
fi
#define CFLAGS "$(expand_libs cflags) -std=gnu++23 -Wall -Wextra -Og -ggdb3"
#define CFLAGS "$(expand_libs cflags) -std=gnu++23 -Wall -Wextra -O0 -ggdb3"
define LDFLAGS "$(expand_libs libs) -L/usr/local/lib -lgrapheme -lboost_filesystem"

cmd rm -rf obj

for file in $files; do
    cmd mkdir -p "obj/$(dirname $file)"
    cmd g++ $CPPFLAGS $CFLAGS -c $file -o "obj/${file%.cpp}.o"
done

cmd g++ $CFLAGS $(find -L obj -name '*.o') -o main $LDFLAGS
#/usr/bin/g++ /usr/local/lib/libgrapheme.a $(find -L obj -name '*.o') -o main
