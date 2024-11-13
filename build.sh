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
    if ! is_defined $1; then
        eval "$1=\"$2\""
    fi
}

function cmd()
{
    echo "$@"
    eval "$@"
}

define CPPFLAGS "-Iinclude"
define CFLAGS "-std=gnu++23 -Wall -Wextra -Og -ggdb3"
define LDFLAGS "-L/usr/local/lib -lgrapheme"

for file in $files; do
    cmd mkdir -p "obj/$(dirname $file)"
    cmd g++ $CPPFLAGS $CFLAGS -c $file -o "obj/${file%.cpp}.o"
done

cmd g++ $CFLAGS $(find -L obj -name '*.o') -o main $LDFLAGS
#/usr/bin/g++ /usr/local/lib/libgrapheme.a $(find -L obj -name '*.o') -o main