#!/bin/sh

RAW=` \
    gcc -E -dM ${1:-Config.h} | \
    sed -n -e '/^.*TLG_FEATURE.*$/d;s/^#define.*\<TLG_\(.*\)\>/\1.c/gp;' \
    | tr '[:upper:]' '[:lower:]' | sort
`
test "${RAW}" != "" ||  exit
cd ${2:-.}
# I added in the extra "ls" so only source files that
# actually exist will show up in the compile list.
ls -1 $RAW 2>/dev/null | sed -e 's/\.c$/\.o/g' | tr '\n' ' '

