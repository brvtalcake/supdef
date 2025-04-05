#!/bin/bash

set -e

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

# given a path, strip the first directory
# e.g. foo/bar/baz -> bar/baz
function strip_first_dir()
{
    printf '%s' "${1#*/}"
}

function map()
{
    local _fn="$1"
    shift
    printf '%s' "$($_fn $1)"
    shift
    for arg in "$@"; do
        printf ' %s' "$($_fn $arg)"
    done
}

function strip_first_dir_if_starts_with()
{
    local _prefix="$1"
    local _path="$2"
    if [[ "$_path" == "$_prefix"* ]]; then
        strip_first_dir "$_path"
    else
        printf '%s' "$_path"
    fi
}

function mk_objfile()
{
    local _file="${1%.cpp}"
    printf 'obj/%s.o' "$(strip_first_dir_if_starts_with src/ $_file)"
}

function cmd()
{
    echo "$@"
    eval "$@"
}

function mk_tmp_jobfile()
{
    local _jobfile="$(mktemp -t parallel.jobfile.XXXXXXXXXX.txt)"
    printf '%s' "$_jobfile"
}

function _replace_pattern()
{
    local _template="$1"
    shift
    local _index="$1"
    shift
    sed -e "s#%$_index#$*#g" <<< "$_template"
}

function replace()
{
    local _template="$1"
    shift
    local _replacement
    local _index=1
    for _replacement in "$@"; do
        _template="$(_replace_pattern "$_template" $_index "$_replacement")"
        _index=$(( _index + 1 ))
    done
    printf '%s' "$_template"
}

function wait_for_user_input()
{
    echo "Press enter to continue..."
    read
}

function cpu_vendor_id()
{
    grep -oPe '(?<=vendor_id\s{0,100}:\s{0,100})\w+' /proc/cpuinfo
}

function cpu_vendor_id_matches()
{
    local _what="$1"
    for vendor_id in $(cpu_vendor_id); do
        if [ "$vendor_id" != "$_what" ]; then
            return 1
        fi
    done
    return 0
}

function cpu_is_intel()
{
    cpu_vendor_id_matches "GenuineIntel"
}

function cpu_is_amd()
{
    cpu_vendor_id_matches "AuthenticAMD"
}

ARGS="$@"
if [ -z "$ARGS" ]; then
    files="$(find -L src -name '*.cpp' -o -name '*.cc' | xargs)"
    produce_executable=1
else
    files="$ARGS"
    produce_executable=0
fi
#objfiles="$(map strip_first_dir_if_starts_with src/ $(find -L src -name '*.cpp' -o -name '*.cc' | xargs))"

jobfile="$(mk_tmp_jobfile)"
trap "rm -f $jobfile" EXIT

common_cflags="-fconcepts-diagnostics-depth=5 -fdiagnostics-color=always -pipe -std=gnu++23 -Wall -Wextra -march=native -mtune=native -flto -Wno-unused-function -Wno-comment -Wno-unused-local-typedefs -Wno-unused-parameter -Wno-unused-label"

define LIBS "icu-io" "libgrapheme" "simdutf" "gmp" "mpfr"

if cpu_is_intel && ! cpu_is_amd; then
    EVE_FLAGS="-DEVE_USE_BMI_ON_AVX2=1 -DEVE_USE_BMI_ON_AVX512=1"
else
    EVE_FLAGS=""
fi
define CPPFLAGS "-Iinclude -I/usr/local/include/eve-2023.2.15 -D_GNU_SOURCE=1 -DSTATIC_INITIALIZER_ALLOCATION=1 -DBOOST_PP_LIMIT_MAG=1024 -DBOOST_PP_LIMIT_FOR=1024 -DBOOST_PP_LIMIT_REPEAT=1024 -DBOOST_PP_LIMIT_ITERATION=1024 $EVE_FLAGS -DMAGIC_ENUM_ENABLE_HASH=1"

if [ -z "$OPTIMIZE" ] || [ "$OPTIMIZE" -eq 0 ]; then
    define CFLAGS "$(expand_libs cflags) $common_cflags -Og -ggdb3"
else
    define CFLAGS "$(expand_libs cflags) $common_cflags -O3"
fi
define LDFLAGS "$(expand_libs libs) -L/usr/local/lib -lgrapheme -lboost_filesystem -lboost_unit_test_framework"

cmd rm -rf obj

for file in $files; do
    cmd mkdir -p "$(dirname "$(mk_objfile $file)")"
    echo -n -e \
        "$(replace 'echo %1 && %1 ;\n' \
            "g++ $CPPFLAGS $CFLAGS -c $file -o \"$(mk_objfile $file)\"" \
        )" >> $jobfile
done

if [ -z "$JOBS" ] || [ "$JOBS" -eq 0 ]; then
    define JOBS "$(nproc)"
fi

cmd parallel --joblog parallel.log -j$JOBS < $jobfile

if [ "$produce_executable" -eq 1 ]; then
    cmd g++ $CFLAGS $(find -L obj -name '*.o') -o main $LDFLAGS
fi

#/usr/bin/g++ /usr/local/lib/libgrapheme.a $(find -L obj -name '*.o') -o main
