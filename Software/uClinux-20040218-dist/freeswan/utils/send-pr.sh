#!/bin/sh
# Submit a problem report to a GNATS site.
# Copyright (C) 2001 Milan Zamazal
# Copyright (C) 1993, 2001 Free Software Foundation, Inc.
# Contributed by Brendan Kehoe (brendan@cygnus.com), based on a
# version written by Heinz G. Seidl (hgs@cygnus.com).
# Further edited by Milan Zamazal (pdm@zamazal.org).
# mktemp support by Yngve Svendsen (yngve.svendsen@clustra.com).
#
# This file is part of GNU GNATS.
#
# GNU GNATS is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# GNU GNATS is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU GNATS; see the file COPYING.  If not, write to
# the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

#
# $Id: send-pr.sh,v 1.3 2001/11/27 15:02:55 mcr Exp $
#

# The version of this send-pr.
VERSION=4.0-alpha

#SWAN_VERSION=

# The submitter-id for your site.
SUBMITTER=net

# The place where our usual binaries live.
BINDIR=@IPSEC_DIR@

# The place where the builtin binaries are located.
LIBEXECDIR=@IPSEC_DIR@

# The default release for this host.
DEFAULT_RELEASE="gnats-4.0-alpha"

# The default organization.
DEFAULT_ORGANIZATION="net"

# How to read the passwd database.
PASSWD="cat /etc/passwd"

# Is the mktemp command available?
MKTEMP="yes"

ECHON=bsd

# By default send-pr connects directly to the database.  However, it
# can be configured to use an existing template file by setting the
# TEMPLATE variable below to point to a PR template generated from
# "send-pr -P".
TEMPLATE="$LIBEXECDIR/ipsec_pr.template"

# send-pr can use mail to submit PRs, instead of connecting to the
# database directly.  MAILPROG needs to point to a compatible mailer
# (sendmail will work).  If MAILPROG needs to have the address that
# the mail is being sent to specified on the command line, it should
# be specified here as well (for example, the command
# MAILPROG="mail bugs@foo.bar.com"
# should work).  If sendmail is used, this should be set to
# MAILPROG="/usr/lib/sendmail -oi -t"
MAILPROG="/usr/sbin/sendmail -oi -t"

# The address that PRs are sent to.  Normally this can be left as "bugs";
# however, if using mail to submit PRs, this should be set to the address
# where PRs should be sent.
MAILADDR="freeswan-bugs@freeswan.org"

if [ $ECHON = bsd ] ; then
  ECHON1="echo -n"
  ECHON2=
elif [ $ECHON = sysv ] ; then
  ECHON1=echo
  ECHON2='\c'
else
  ECHON1=echo
  ECHON2=
fi

# Configuration file to be read.  It must be a shell script that can redefine
# the variables above to fit a local configuration.
CONFIGFILE=@IPSEC_DIR@/send-pr.conf

if [ -r $CONFIGFILE ]; then
  . $CONFIGFILE
fi

#

if [ -z "$TMPDIR" ]; then
  TMPDIR=/tmp
else
  if [ "`echo $TMPDIR | grep '/$'`" != "" ]; then
    TMPDIR="`echo $TMPDIR | sed -e 's,/$,,'`"
  fi
fi

# TEMP:   Temporary copy of the PR, to be edited by the user.
# BAD:    The PR will end up here if the user aborts.
# REF:    The 'reference' copy of the PR template, used to verify that the user
#         actually did edit the template.
# FIXFIL: A sed script used to remove comments from the template before
#         processing.
if [ $MKTEMP = yes ]; then
  TEMP=`mktemp $TMPDIR/pXXXXXX` || exit 1
  BAD=`mktemp $TMPDIR/pbadXXXXXX` || exit 1
  REF=`mktemp $TMPDIR/pfXXXXXX` || exit 1
  FIXFIL=`mktemp $TMPDIR/fixXXXXXX` || exit 1
else
  TEMP=$TMPDIR/p$$
  BAD=$TMPDIR/pbad$$
  REF=$TMPDIR/pf$$
  FIXFIL=$TMPDIR/fix$$
  bad_temp=0
  : > $TEMP || bad_temp=1
  : > $BAD || bad_temp=1
  : > $REF || bad_temp=1
  : > $FIXFIL || bad_temp=1
  if [ $bad_temp = 1 ]; then
      rm -f $TEMP $BAD $REF $FIXFIL
      exit 1;
  fi
fi
REMOVE_TEMP="rm -f $TEMP $BAD $REF"

# find a user name
if [ "$LOGNAME" = "" ]; then
	if [ "$USER" != "" ]; then
		LOGNAME="$USER"
	else
		LOGNAME="UNKNOWN"
	fi
fi

FROM="$LOGNAME"
REPLYTO="${REPLY_TO:-${REPLYTO:-$LOGNAME}}"
if [ "x$MAILPROG" != "x" ]
then
    RESP_ALIAS="`query-pr --adm-field responsible --adm-key $LOGNAME --adm-subfield alias 2>/dev/null`"
else
    RESP_ALIAS=""
fi

# Find out the name of the originator of this PR.
if [ -n "$NAME" ]; then
  DEFAULT_ORIGINATOR="$NAME"
elif [ -f $HOME/.fullname ]; then
  DEFAULT_ORIGINATOR="`sed -e '1q' $HOME/.fullname`"
else
  # Must use temp file due to incompatibilities in quoting behavior
  # and to protect shell metacharacters in the expansion of $LOGNAME
  $PASSWD | grep "^$LOGNAME:" | awk -F: '{print $5}' | sed -e 's/,.*//' > $TEMP
  if [ "x$RESP_ALIAS" != "x" ]
  then
    DEFAULT_ORIGINATOR="$RESP_ALIAS (`cat $TEMP`)"
  else
    DEFAULT_ORIGINATOR="$FROM (`cat $TEMP`)"
  fi
  rm -f $TEMP
fi

if [ -z "$ORGANIZATION" ]
then
  ORGANIZATION="$DEFAULT_ORGANIZATION";
fi

if [ -n "$ORGANIZATION" -a "x$ORGANIZATION" != "xunknown" ]; then
  if [ -f "$ORGANIZATION" ]; then
    ORGANIZATION="`cat $ORGANIZATION`"
  fi
  if [ -n "$ORGANIZATION" ]; then
    ORGANIZATION="$ORGANIZATION"
  elif [ -f $HOME/.organization ]; then
    ORGANIZATION="`cat $HOME/.organization`"
  fi
fi

if [ "x$ORGANIZATION" = "xunknown" ]; then
  cat <<__EOF__
It seems that send-pr is not installed with your organization set to a useful
value.  To fix this, you need to edit the configuration file
$CONFIGFILE
and fill in the organization with the correct value.

__EOF__
  ORGANIZATION="";
fi 1>&2

# If they don't have a preferred editor set, then use
if [ -z "$VISUAL" ]; then
  if [ -z "$EDITOR" ]; then
    EDIT=vi
  else
    EDIT="$EDITOR"
  fi
else
  EDIT="$VISUAL"
fi

# Find out some information.
SYSTEM=`( [ -f /bin/uname ] && /bin/uname -a ) || \
        ( [ -f /usr/bin/uname ] && /usr/bin/uname -a ) || echo ""`

# Our base command name.
COMMAND=`echo $0 | sed -e 's,.*/,,'`
USAGE="Usage: $COMMAND [OPTION]...

  -b --batch              run without printing most messages
     --barf               include a full barf inline rather than just look
  -c --cc=LINE            put LINE to the CC header
  -d --database=DATABASE  submit PR to DATABASE
  -f --file=FILE          read the PR template from FILE (\`-' for stdin)
  -p --print              just print the template and exit
     --request-id         send a request for a user id
  -s --severity=SEVERITY  PR severity
  
  -h --help               display this help and exit
  -V --version            output version information and exit
"
REMOVE=
BATCH=
CC=
DEFAULT_SEVERITY=
BARF=${BARF-false}

if [ "$SYSTEM" != "" ]
then
    DEFAULT_ENVIRONMENT="System: $SYSTEM"
fi

if [ "$SWAN_VERSION" != "" ]
then
    DEFAULT_VERSION="$SWAN_VERSION";
else
    DEFAULT_VERSION=`ipsec --versioncode`
fi
DEFAULT_VERSION=`echo $DEFAULT_VERSION | sed -e 's/\//\\\//'`

while [ $# -gt 0 ]; do
  case "$1" in
    -r) ;; 		# Ignore for backward compat.
    -f | --file) if [ $# -eq 1 ]; then echo "$USAGE"; exit 1; fi
	shift ; IN_FILE="$1"
	if [ "$IN_FILE" != "-" -a ! -r "$IN_FILE" ]; then
	  echo "$COMMAND: cannot read $IN_FILE"
	  exit 1
	fi
	;;
    -b | --batch) BATCH=true ;;
    --barf) BARF=true ;;
    -c | --cc) if [ $# -eq 1 ]; then echo "$USAGE"; exit 1; fi
	shift ; CC="$1"
	;;
    -d | --database) if [ $# -eq 1 ]; then echo "$USAGE"; exit 1; fi
        shift; GNATSDB="$1"; export GNATSDB
    ;;
    -s | --severity) if [ $# -eq 1 ]; then echo "$USAGE"; exit 1; fi
	shift ; DEFAULT_SEVERITY="$1"
	;;
    -p | -P | --print) PRINT=true ;;
    --request-id) REQUEST_ID=true ;;
    -h | --help) echo "$USAGE"; exit 0 ;;
    -V | --version) echo "$VERSION"; exit 0 ;;
    -*) echo "$USAGE" ; exit 1 ;;
    *)  echo "$USAGE" ; exit 1 ;;
 esac
 shift
done

if [ "x$SUBMITTER" = "x" ]
then
  SUBMITTER="unknown"
fi

if [ "x$SUBMITTER" = "xunknown" -a -z "$REQUEST_ID" -a -z "$IN_FILE" ]; then
  cat << '__EOF__'
It seems that send-pr is not installed with your unique submitter-id.
You need to run

          install-sid YOUR-SID

where YOUR-SID is the identification code you received with `send-pr'.
`send-pr' will automatically insert this value into the template field
`>Submitter-Id'.  If you've downloaded `send-pr' from the Net, use `net'
for this value.  If you do not know your id, run `send-pr --request-id' to 
get one from your support site.
__EOF__
  exit 1
fi

# So the template generation code finds it.
DEFAULT_SUBMITTERID=${SUBMITTER}

# Catch some signals. ($xs kludge needed by Sun /bin/sh)
xs=0
trap 'rm -f $REF $TEMP $FIXFIL; exit $xs' 0
trap 'echo "$COMMAND: Aborting ..."; rm -f $REF $TEMP $FIXFIL; xs=1; exit' 1 3 13 15

if [ "x$PRINT" = "xtrue" ]; then
  FROM="<FROM>"
  REPLYTO="<REPLYTO>"
  DEFAULT_ORIGINATOR="<DEFAULT_ORIGINATOR>"
  DEFAULT_SUBMITTERID="<SUBMITTER>"
fi

# If they told us to use a specific file, then do so.
if [ -n "$IN_FILE" ]; then
  if [ "$IN_FILE" = "-" ]; then
    # The PR is coming from the standard input.
    cat > $TEMP
  else
    # Use the file they named.
    cat $IN_FILE > $TEMP
  fi
else
  if [ -n "$TEMPLATE" -a -z "$PRINT_INTERN" ]; then
    # If their TEMPLATE points to a bogus entry, then bail.
    if [ ! -f "$TEMPLATE" -o ! -r "$TEMPLATE" -o ! -s "$TEMPLATE" ]; then
      echo "$COMMAND: can't seem to read your template file (\`$TEMPLATE'), ignoring TEMPLATE"
      sleep 1
      PRINT_INTERN=bad_prform
    fi
  fi

  if [ -n "$TEMPLATE" -a -z "$PRINT_INTERN" ]; then
    sed "s/<FROM>/$FROM/;s/<REPLYTO>/$REPLYTO/;s/<DEFAULT_ORIGINATOR>/$DEFAULT_ORIGINATOR/;s/<SUBMITTER>/$DEFAULT_SUBMITTERID/;s/<DEFAULT_ENVIRONMENT>/$DEFAULT_ENVIRONMENT/;s/<DEFAULT_BARF>/$DEFAULT_BARF/;s/<DEFAULT_VERSION>/$DEFAULT_VERSION/;" < $TEMPLATE > $TEMP ||
      ( echo "$COMMAND: could not copy $TEMPLATE" ; xs=1; exit )
  else
    # Which genius thought of iterating through this loop twice, when the
    # cp command would suffice?
    for file in $TEMP ; do
      cat  > $file << '__EOF__'
SEND-PR: -*- send-pr -*-
SEND-PR: Lines starting with `SEND-PR' will be removed automatically, as
SEND-PR: will all comments (text enclosed in `<' and `>').
SEND-PR: 
SEND-PR: Please consult the send-pr man page `send-pr(1)' or the Texinfo
SEND-PR: manual if you are not sure how to fill out a problem report.
SEND-PR: Note that the Synopsis field is mandatory.  The Subject (for
SEND-PR: the mail) will be made the same as Synopsis unless explicitly
SEND-PR: changed.
SEND-PR:
SEND-PR: Choose from the following categories:
SEND-PR:
__EOF__

      # Format the categories so they fit onto lines.
        CATEGORIES=`${BINDIR}/query-pr --valid-values Category`;
	l=`echo "$CATEGORIES" | \
	awk 'BEGIN {max = 0; } { if (length($0) > max) { max = length($0); } }
	     END {print max + 1;}'`
	c=`expr 61 / $l`
	if [ $c -eq 0 ]; then c=1; fi
	echo "$CATEGORIES" | \
        awk 'BEGIN {printf "SEND-PR: "; i = 0 }
          { printf ("%-'$l'.'$l's", $0);
	    if ((++i % '$c') == 0) { printf "\nSEND-PR: " } }
            END { printf "\nSEND-PR:\n"; }' >> $file

	cat >> $file << __EOF__
To: $MAILADDR
Subject: 
From: $FROM
Reply-To: $REPLYTO
Cc: $CC
X-send-pr-version: $VERSION
X-GNATS-Notify: 


__EOF__

	#
	# Iterate through the list of input fields.  fieldname is the
	# name of the field.  fmtname is the formatted name of the field,
	# with >, : and extra spaces to cause the field contents to be
	# aligned.
	#
	${BINDIR}/query-pr --list-input-fields | awk '{a[NR]=$1""; mnr = NR+1; len = length($1) + 2; if (mlen < len) mlen = len; } END { for (x = 1; x < mnr; x++) { b = ">"a[x]":"; printf ("%s %-"mlen"s&\n", a[x], b); } }' |  while read fieldname fmtname
	do
	    fmtname="`echo "$fmtname" | sed 's/[&]$//;'`"
	    upname="`echo $fieldname | sed 'y/abcdefghijklmnopqrstuvwxyz/ABCDEFGHIJKLMNOPQRSTUVWXYZ/;s/-//g;'`"
	    # Grab the default value for this field.
	    eval 'default_val="$DEFAULT_'${upname}'"'
	    # What's stored in the field?
	    type=`${BINDIR}/query-pr --field-type $fieldname | sed 'y/ABCDEFGHIJKLMNOPQRSTUVWXYZ/abcdefghijklmnopqrstuvwxyz/'`
	    case $type in
		enum)
		    if [ "$default_val" != "" ]
		    then
			desc=$default_val;
		    else
			if [ "$fieldname" != "Category" ]
			then
			    values=`${BINDIR}/query-pr --valid-values $fieldname | tr '\n' ' ' | sed 's/ *$//g; s/ / | /g;s/^/[ /;s/$/ ]/;'`
			    valslen=`echo "$values" | wc -c`
			else
			    values="choose from a category listed above"
			    valslen=1;
			fi
			if [ "$valslen" -gt 160 ]
			then
				desc="<`${BINDIR}/query-pr --field-description $fieldname` (one line)>";
			else
				desc="<${values} (one line)>";
			fi
			dpat=`echo "$desc" | tr '\]\[*+^$|\()&/' '............'`
			echo "/^>${fieldname}:/ s/${dpat}//" >> $FIXFIL
		    fi
		    echo "${fmtname}${desc}" >> $file
		    ;;
		multitext)
		    if [ "$default_val" != "" ]
		    then
			desc="	$default_val";
		    else
		        desc="	<`${BINDIR}/query-pr --field-description $fieldname` (multiple lines)>";
			dpat=`echo "$desc" | tr '\]\[*+^$|\()&/' '............'`
			echo "s/^${dpat}//" >> $FIXFIL
		    fi
		    echo "${fmtname}" >> $file;
		    echo "$desc" >> $file;
		    ;;
		*)
		    if [ "$default_val" != "" ]
		    then
			desc="${default_val}"
		    else
			desc="<`${BINDIR}/query-pr --field-description $fieldname` (one line)>"
			dpat=`echo "$desc" | tr '\]\[*+^$|\()&/' '............'`
			echo "/^>${fieldname}:/ s/${dpat}//" >> $FIXFIL
		    fi
		    echo "${fmtname}${desc}" >> $file
		    ;;
	    esac
	done
    done
  fi

  if [ "$PRINT" = true -o "$PRINT_INTERN" = true ]; then
    cat $TEMP
    xs=0; exit
  fi

  if $BARF
  then
    ipsec barf >>$TEMP
  else
    ipsec look >>$TEMP
  fi

  cp $TEMP $REF

  chmod u+w $TEMP
  if [ -z "$REQUEST_ID" ]; then
    eval $EDIT $TEMP
  else
    ed -s $TEMP << '__EOF__'
/^Subject/s/^Subject:.*/Subject: request for a customer id/
/^>Category/s/^>Category:.*/>Category: send-pr/
w
q
__EOF__
  fi

  if cmp -s $REF $TEMP ; then
    echo "$COMMAND: problem report not filled out, therefore not sent"
    xs=1; exit
  fi
fi

# TEMP is the PR that we are editing.  When we're done, REF will contain
# the final PR to be sent.

while [ -z "$REQUEST_ID" ]; do
  CNT=0

  #
  #	Remove comments.
  #
  echo '/^SEND-PR:/d' >> $FIXFIL
  sed -f $FIXFIL $TEMP > $REF

  # REF now has the actual PR that we want to send.

  #
  # Check that synopsis is not empty.
  #
  if grep "^>Synopsis:[ 	]*$" $REF > /dev/null
  then
    echo "$COMMAND: Synopsis must not be empty."
    CNT=`expr $CNT + 1`
  fi

  if [ "x$MAILPROG" = "x" ]
  then
    # Since we're not using mail, use pr-edit to check the PR.  We can't
    # do much checking otherwise, sorry.
    $LIBEXECDIR/pr-edit --check-initial < $REF || CNT=`expr $CNT + 1`
  fi

  [ $CNT -gt 0 -a -z "$BATCH" ] && 
    echo "Errors were found with the problem report."

  while true; do
    if [ -z "$BATCH" ]; then
      $ECHON1 "a)bort, e)dit or s)end? $ECHON2"
      read input
    else
      if [ $CNT -eq 0 ]; then
        input=s
      else
        input=a
      fi
    fi
    case "$input" in
      a*)
	if [ -z "$BATCH" ]; then
	  echo "$COMMAND: the problem report remains in $BAD and is not sent."
	  mv $TEMP $BAD
        else
	  echo "$COMMAND: the problem report is not sent."
	fi
	xs=1; exit
	;;
      e*)
        eval $EDIT $TEMP
	continue 2
	;;
      s*)
	break 2
	;;
    esac
  done
done

#
# Make sure the mail has got a Subject.  If not, use the same as
# in Synopsis.
#

if grep '^Subject:[ 	]*$' $REF > /dev/null
then
  SYNOPSIS=`grep '^>Synopsis:' $REF | sed -e 's/^>Synopsis:[ 	]*//'`
  ed -s $REF << __EOF__
/^Subject:/s/:.*\$/: $SYNOPSIS/
w
q
__EOF__
fi

while :
do
  if [ "x$MAILPROG" != "x" ]
  then
    # Use mail to send the PR.
    $MAILPROG < $REF;
    echo "$COMMAND: problem report mailed"
    xs=0; exit
  else
    if $LIBEXECDIR/pr-edit --submit < $REF; then
      echo "$COMMAND: problem report filed"
      xs=0; exit
    else
      echo "$COMMAND: the problem report is not sent."
    fi
  fi
  while true
  do
    if [ -z "$BATCH" ]; then
      $ECHON1 "a)bort or s)end? $ECHON2"
      read input
      case "$input" in
        a*)
	  break 2 ;;
        s*)
	  break ;;
      esac
    else
      break 2;
    fi
  done
done

if [ -z "$BATCH" ]; then
  echo "$COMMAND: the problem report remains in $BAD and is not sent."
  mv $TEMP $BAD
else
  echo "$COMMAND: the problem report is not sent."
fi

xs=1; exit;

#
# $Log: send-pr.sh,v $
# Revision 1.3  2001/11/27 15:02:55  mcr
# 	added rcsids.
# 	fixed submission address to be freeswan-bugs@freeswan.org
# 	use new ipsec --versioncode to get version info.
#
#
