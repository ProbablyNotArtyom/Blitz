#! @BASH@
# Copyright (C) 1999, 2000, 2001, 2002 Free Software Foundation, Inc.
# This file is part of the GNU C Library.
# Contributed by Ulrich Drepper <drepper@gnu.org>, 1999.

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.

# You should have received a copy of the GNU Lesser General Public
# License along with the GNU C Library; if not, write to the Free
# Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# 02111-1307 USA.

memusageso=@SLIBDIR@/libmemusage.so
memusagestat=@BINDIR@/memusagestat

# Print usage message.
do_usage() {
  echo >&2 $"Try \`memusage --help' for more information."
  exit 1
}

# Message for missing argument.
do_missing_arg() {
  echo >&2 $"memusage: option \`$1' requires an argument"
  do_usage
}

# Print help message
do_help() {
  echo $"Usage: memusage [OPTION]... PROGRAM [PROGRAMOPTION]...
Profile memory usage of PROGRAM.

   -n,--progname=NAME     Name of the program file to profile
   -p,--png=FILE          Generate PNG graphic and store it in FILE
   -d,--data=FILE         Generate binary data file and store it in FILE
   -u,--unbuffered        Don't buffer output
   -b,--buffer=SIZE       Collect SIZE entries before writing them out
      --no-timer          Don't collect additional information though timer

   -?,--help              Print this help and exit
      --usage             Give a short usage message
   -V,--version           Print version information and exit

 The following options only apply when generating graphical output:
   -t,--time-based        Make graph linear in time
   -T,--total             Also draw graph of total memory use
      --title=STRING      Use STRING as title of the graph
   -x,--x-size=SIZE       Make graphic SIZE pixels wide
   -y,--y-size=SIZE       Make graphic SIZE pixels high

Mandatory arguments to long options are also mandatory for any corresponding
short options.

Report bugs using the \`glibcbug' script to <bugs@gnu.org>."
  exit 0
}

do_version() {
  echo 'memusage (GNU libc) @VERSION@'
  echo $"Copyright (C) 2002 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
Written by Ulrich Drepper."
  exit 0
}

# Process arguments.  But stop as soon as the program name is found.
while test $# -gt 0; do
  case "$1" in
  -V | --v | --ve | --ver | --vers | --versi | --versio | --version)
    do_version
    ;;
  -\? | --h | --he | --hel | --help)
    do_help
    ;;
  --us | --usa | --usag | --usage)
    echo $"Syntax: memusage [--data=FILE] [--progname=NAME] [--png=FILE] [--unbuffered]
            [--buffer=SIZE] [--no-timer] [--time-based] [--total]
            [--title=STRING] [--x-size=SIZE] [--y-size=SIZE]
            PROGRAM [PROGRAMOPTION]..."
    exit 0
    ;;
  -n | --pr | --pro | --prog | --progn | --progna | --prognam | --progname)
    if test $# -eq 1; then
      do_missing_arg $1
    fi
    shift
    progname="$1"
    ;;
  --pr=* | --pro=* | --prog=* | --progn=* | --progna=* | --prognam=* | --progname=*)
    progname=${1##*=}
    ;;
  -p | --pn | --png)
    if test $# -eq 1; then
      do_missing_arg $1
    fi
    shift
    png="$1"
    ;;
  --pn=* | --png=*)
    png=${1##*=}
    ;;
  -d | --d | --da | --dat | --data)
    if test $# -eq 1; then
      do_missing_arg $1
    fi
    shift
    data="$1"
    ;;
  --d=* | --da=* | --dat=* | --data=*)
    data=${1##*=}
    ;;
  -u | --un | --unb | --unbu | --unbuf | --unbuff | --unbuffe | --unbuffer | --unbuffere | --unbuffered)
    buffer=1
    ;;
  -b | --b | --bu | --buf | --buff | --buffe | --buffer)
    if test $# -eq 1; then
      do_missing_arg $1
    fi
    shift
    buffer="$1"
    ;;
  --b=* | --bu=* | --buf=* | --buff=* | --buffe=* | --buffer=*)
    buffer=${1##*=}
    ;;
  --n | --no | --no- | --no-t | --no-ti | --no-tim | --no-time | --no-timer)
    notimer=yes
    ;;
  -t | --tim | --time | --time- | --time-b | --time-ba | --time-bas | --time-base | --time-based)
    memusagestat_args="$memusagestat_args -t"
    ;;
  -T | --to | --tot | --tota | --total)
    memusagestat_args="$memusagestat_args -T"
    ;;
  --tit | --titl | --title)
    if test $# -eq 1; then
      do_missing_arg $1
    fi
    shift
    memusagestat_args="$memusagestat_args -s $1"
    ;;
  --tit=* | --titl=* | --title=*)
    memusagestat_args="$memusagestat_args -s ${1##*=}"
    ;;
  -x | --x | --x- | --x-s | --x-si | --x-siz | --x-size)
    if test $# -eq 1; then
      do_missing_arg $1
    fi
    shift
    memusagestat_args="$memusagestat_args -x $1"
    ;;
  --x=* | --x-=* | --x-s=* | --x-si=* | --x-siz=* | --x-size=*)
    memusagestat_args="$memusagestat_args -x ${1##*=}"
    ;;
  -y | --y | --y- | --y-s | --y-si | --y-siz | --y-size)
    if test $# -eq 1; then
      do_missing_arg $1
    fi
    shift
    memusagestat_args="$memusagestat_args -y $1"
    ;;
  --y=* | --y-=* | --y-s=* | --y-si=* | --y-siz=* | --y-size=*)
    memusagestat_args="$memusagestat_args -y ${1##*=}"
    ;;
  --p | --p=* | --t | --t=* | --ti | --ti=* | --u)
    echo >&2 $"memusage: option \`${1##*=}' is ambiguous"
    do_usage
    ;;
  --)
    # Stop processing arguments.
    shift
    break
    ;;
  --*)
    echo >&2 $"memusage: unrecognized option \`$1'"
    do_usage
    ;;
  *)
    # Unknown option.  This means the rest is the program name and parameters.
    break
    ;;
  esac
  shift
done

# See whether any arguments are left.
if test $# -eq 0; then
  echo >&2 $"No program name given"
  do_usage
fi

# This will be in the environment.
add_env="LD_PRELOAD=$memusageso"

# Generate data file name.
datafile=
if test -n "$data"; then
  datafile="$data"
elif test -n "$png"; then
  datafile=$(mktemp ${TMPDIR:-/tmp}/memusage.XXXXXX 2> /dev/null)
  if test $? -ne 0; then
    # Lame, but if there is no `mktemp' program the user cannot expect more.
    if test "$RANDOM" != "$RANDOM"; then
      datafile=${TMPDIR:-/tmp}/memusage.$RANDOM
    else
      datafile=${TMPDIR:-/tmp}/memusage.$$
    fi
  fi
fi
if test -n "$datafile"; then
  add_env="$add_env MEMUSAGE_OUTPUT=$datafile"
fi

# Set program name.
if test -n "$progname"; then
  add_env="$add_env MEMUSAGE_PROG_NAME=$progname"
fi

# Set buffer size.
if test -n "$buffer"; then
  add_env="$add_env MEMUSAGE_BUFFER_SIZE=$buffer"
fi

# Disable timers.
if test -n "$notimer"; then
  add_env="$add_env MEMUSAGE_NO_TIMER=yes"
fi

# Execute the program itself.
eval $add_env '"$@"'
result=$?

# Generate the PNG data file if wanted and there is something to generate
# it from.
if test -n "$png" -a -n "$datafile" -a -s "$datafile"; then
  # Append extension .png if it isn't already there.
  case $png in
  *.png) ;;
  *) png="$png.png" ;;
  esac
  $memusagestat $memusagestat_args "$datafile" "$png"
fi

if test -z "$data" -a -n "$datafile"; then
  rm -f "$datafile"
fi

exit $result
# Local Variables:
#  mode:ksh
# End:
