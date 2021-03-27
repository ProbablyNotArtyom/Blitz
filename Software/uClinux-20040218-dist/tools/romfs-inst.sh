#!/bin/sh
#
# A tool to simplify Makefiles that need to put something
# into the ROMFS
#
# Copyright (C) David McCullough, 2002,2003
#
#############################################################################

# Provide a default PATH setting to avoid potential problems...
PATH="/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:$PATH"

usage()
{
cat << !EOF >&2
$0: [options] [src] dst
    -v          : output actions performed.
    -e env-var  : only take action if env-var is set to "y".
    -o option   : only take action if option is set to "y".
    -p perms    : chmod style permissions for dst.
    -a text     : append text to dst.
	-A pattern  : only append text if pattern doesn't exist in file
    -l link     : dst is a link to 'link'.
    -s sym-link : dst is a sym-link to 'sym-link'.

    if "src" is not provided,  basename is run on dst to determine the
    source in the current directory.

	multiple -e and -o options are ANDed together.  To achieve an OR affect
	use a single -e/-o with 1 or more y/n/"" chars in the condition.

	if src is a directory,  everything in it is copied recursively to dst
	with special files removed (currently CVS dirs).
!EOF
	exit 1
}

#############################################################################

setperm()
{
	if [ "$perm" ]
	then
		[ "$v" ] && echo "chmod ${perm} ${ROMFSDIR}${dst}"
		chmod ${perm} ${ROMFSDIR}${dst}
	fi
}

#############################################################################

file_copy()
{
	if [ -d "${src}" ]
	then
		[ "$v" ] && echo "CopyDir ${src} ${ROMFSDIR}${dst}"
		(
			cd ${src}
			V=
			[ "$v" ] && V=v
			find . -print | grep -E -v '/CVS' | cpio -p${V}dumL ${ROMFSDIR}${dst}
		)
	else
		rm -f ${ROMFSDIR}${dst}
		[ "$v" ] && echo "cp ${src} ${ROMFSDIR}${dst}"
		cp ${src} ${ROMFSDIR}${dst}
		setperm ${ROMFSDIR}${dst}
	fi
}

#############################################################################

file_append()
{
	touch ${ROMFSDIR}${dst}
	if [ -z "${pattern}" ] && grep -F "${src}" ${ROMFSDIR}${dst} > /dev/null
	then
		[ "$v" ] && echo "File entry already installed."
	elif [ "${pattern}" ] && egrep "${pattern}" ${ROMFSDIR}${dst} > /dev/null
	then
		[ "$v" ] && echo "File pattern already installed."
	else
		[ "$v" ] && echo "Installing entry into ${ROMFSDIR}${dst}."
		echo "${src}" >> ${ROMFSDIR}${dst}
	fi
	setperm ${ROMFSDIR}${dst}
}

#############################################################################

hard_link()
{
	rm -f ${ROMFSDIR}${dst}
	[ "$v" ] && echo "ln ${src} ${ROMFSDIR}${dst}"
	ln ${src} ${ROMFSDIR}${dst}
}

#############################################################################

sym_link()
{
	rm -f ${ROMFSDIR}${dst}
	[ "$v" ] && echo "ln -s ${src} ${ROMFSDIR}${dst}"
	ln -s ${src} ${ROMFSDIR}${dst}
}

#############################################################################
#
# main program entry point
#

if [ -z "$ROMFSDIR" ]
then
	echo "ROMFSDIR is not set" >&2
	usage
	exit 1
fi

v=
option=y
pattern=
perm=
func=file_copy
src=
dst=

while getopts 've:o:A:p:a:l:s:' opt "$@"
do
	case "$opt" in
	v) v="1";                           ;;
	o) option="$OPTARG";                ;;
	e) eval option=\"\$$OPTARG\";       ;;
	p) perm="$OPTARG";                  ;;
	a) src="$OPTARG"; func=file_append; ;;
	A) pattern="$OPTARG";               ;;
	l) src="$OPTARG"; func=hard_link;   ;;
	s) src="$OPTARG"; func=sym_link;    ;;

	*)  break ;;
	esac
#
#	process option here to get an ANDing effect
#
	case "$option" in
	*[yY]*) # this gives OR effect, ie., nYn
		;;
	*)
		[ "$v" ] && echo "Condition not satisfied."
		exit 0
		;;
	esac
done

shift `expr $OPTIND - 1`

case $# in
1)
	dst="$1"
	if [ -z "$src" ]
	then
		src="`basename $dst`"
	fi
	;;
2)
	if [ ! -z "$src" ]
	then
		echo "Source file already provided" >&2
		exit 1
	fi
	src="$1"
	dst="$2"
	;;
*)
	usage
	;;
esac

$func

exit 0

#############################################################################
