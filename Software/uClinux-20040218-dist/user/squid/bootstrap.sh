#!/bin/sh
# Used to setup the configure.in, autoheader and Makefile.in's if configure
# has not been generated. This script is only needed for developers when
# configure has not been run, or if a Makefile.am in a non-configured directory
# has been updated

# Autotool versions preferred. To override either edit the script
# to match the versions you want to use, or set the variables on
# the command line like "env acver=.. amver=... ./bootstrap.sh"

acversions="${acver:-2.13}"
amversions="${amver:-1.5}"

check_version()
{
  eval $2 --version 2>/dev/null | grep -i "$1.*$3" >/dev/null
}

find_version()
{
  tool=$1
  found="NOT_FOUND"
  shift
  versions="$*"
  for version in $versions; do
    for variant in "" "-${version}" "`echo $version | sed -e 's/\.//g'`"; do
      if check_version $tool ${tool}${variant} $version; then
	found="${variant}"
	break
      fi
    done
    if [ "x$found" != "xNOT_FOUND" ]; then
      break
    fi
  done
  if [ "x$found" = "xNOT_FOUND" ]; then
    echo "WARNING: Cannot find $tool version $versions" >&2
    echo "Trying `$tool --version | head -1`" >&2
    found=""
  fi
  echo $found
}

bootstrap() {
  if "$@"; then
    true # Everything OK
  else
    echo "$1 failed"
    echo "Autotool bootstrapping failed. You will need to investigate and correct" ;
    echo "before you can develop on this source tree" 
    exit 1
  fi
}

# Make sure cfgaux exists
mkdir -p cfgaux

# Adjust paths of required autool packages
amver=`find_version automake ${amversions}`
acver=`find_version autoconf ${acversions}`

# Bootstrap the autotool subsystems
bootstrap aclocal$amver
bootstrap autoheader$acver
bootstrap automake$amver --foreign --add-missing
bootstrap autoconf$acver

echo "Autotool bootstrapping complete."
