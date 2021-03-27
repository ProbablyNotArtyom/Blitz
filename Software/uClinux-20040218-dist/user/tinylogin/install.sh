#!/bin/sh

prefix=$1
if [ "$prefix" = "" ]; then
    echo "No installation directory, aborting."
    exit 1;
fi
if [ "$2" = "--hardlinks" ]; then
    linkopts="-f"
else
    linkopts="-fs"
fi
h=`sort tinylogin.links | uniq`


rm -f $prefix/bin/tinylogin || exit 1
mkdir -p $prefix/bin || exit 1
install -m 755 tinylogin $prefix/bin/tinylogin || exit 1

for i in $h ; do
	appdir=`dirname $i`
	mkdir -p $prefix/$appdir || exit 1
	if [ "$2" = "--hardlinks" ]; then
	    bb_path="$prefix/bin/tinylogin"
	else
	    case "$appdir" in
		/)
		    bb_path="bin/tinylogin"
		;;
		/bin)
		    bb_path="tinylogin"
		;;
		/sbin)
		    bb_path="../bin/tinylogin"
		;;
		/usr/bin|/usr/sbin)
		    bb_path="../../bin/tinylogin"
		;;
		*)
		echo "Unknown installation directory: $appdir"
		exit 1
		;;
	    esac
	fi
	echo "  $prefix$i -> $bb_path"
	ln $linkopts $bb_path $prefix$i || exit 1
done

exit 0
