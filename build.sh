#!/bin/bash

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

define LIBS "icu-io" "libgrapheme"
define CPPFLAGS "-Iinclude -D_GNU_SOURCE=1"
define CFLAGS "$(expand_libs cflags) -std=gnu++23 -Wall -Wextra -Og -ggdb3"
define LDFLAGS "$(expand_libs libs) -L/usr/local/lib -lgrapheme"

cmd rm -rf obj

for file in $files; do
    cmd mkdir -p "obj/$(dirname $file)"
    cmd g++ $CPPFLAGS $CFLAGS -c $file -o "obj/${file%.cpp}.o"
done

cmd g++ $CFLAGS $(find -L obj -name '*.o') -o main $LDFLAGS
#/usr/bin/g++ /usr/local/lib/libgrapheme.a $(find -L obj -name '*.o') -o main