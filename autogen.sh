#!/bin/sh
#
# autogen.sh - Generates the initial makefiles from a pristine CVS tree
#
# $Id: autogen.sh,v 1.1 2001/05/16 15:22:25 cmdjb Exp $
#
# USAGE: autogen.sh [configure options]
#
# If environment variable DRYRUN is set, no configuring will be done -
# (e.g. in bash)  DRYRUN=1 ./autogen.sh
# will not do any configuring but will emit the programs that would be run.
#
# This script is based on similar scripts used in various tools
# commonly made available via CVS and used with GNU automake.
# Try 'locate autogen.sh' on your system and see what you get.
#

PACKAGE=pngmeta
DIE=

if test "X$DRYRUN" != X; then
  DRYRUN=echo
fi

autoconf_vers=2.13
automake_vers=1.4
aclocal_vers=1.4

program=`basename $0`

if (autoconf --version) < /dev/null > /dev/null 2>&1 ; then
    if (autoconf --version | awk 'NR==1 { if( $3 >= '$autoconf_vers') \
			       exit 1; exit 0; }');
    then
       echo "$program: ERROR: \`autoconf' is too old."
       echo "           (version $autoconf_vers or newer is required)"
       DIE="yes"
    fi
else
    echo
    echo "$program: ERROR: You must have \`autoconf' installed to compile $PACKAGE."
    echo "           (version $autoconf_vers or newer is required)"
    DIE="yes"
fi


if (automake --version) < /dev/null > /dev/null 2>&1 ; then
  if (automake --version | awk 'NR==1 { if( $4 >= '$automake_vers') \
			     exit 1; exit 0; }');
     then
     echo "$program: ERROR: \`automake' is too old."
     echo "           (version $automake_vers or newer is required)"
     DIE="yes"
  fi
  if (aclocal --version) < /dev/null > /dev/null 2>&1; then
    if (aclocal --version | awk 'NR==1 { if( $4 >= '$aclocal_vers' ) \
						exit 1; exit 0; }' );
    then
      echo "$program: ERROR: \`aclocal' is too old."
      echo "           (version $aclocal_vers or newer is required)"
      DIE="yes"
    fi
  else
    echo
    echo "$program: ERROR: Missing \`aclocal'"
    echo "           The version of automake installed doesn't appear recent enough."
    DIE="yes"
  fi
else
    echo
    echo "$program: ERROR: You must have \`automake' installed to compile $PACKAGE."
    echo "           (version $automake_vers or newer is required)"
    DIE="yes"
fi


if test "X$DIE" != X; then
  exit 1
fi

if test -z "$*"; then
  echo "$program: WARNING: Running \`configure' with no arguments."
  echo "If you wish to pass any to it, please specify them on the"
  echo "\`$0' command line."
fi

am_opt=

for coin in `find $srcdir -name configure.in -print`
do 
  dir=`dirname $coin`
  if test -f $dir/NO-AUTO-GEN; then
    echo $program: Skipping $dir -- flagged as no auto-gen
  else
    echo $program: Processing directory $dir
    ( cd $dir
      aclocalinclude="$ACLOCAL_FLAGS"
      echo "$program: Running aclocal $aclocalinclude"
      $DRYRUN aclocal $aclocalinclude
      if grep "^AM_CONFIG_HEADER" configure.in >/dev/null; then
	echo "$program: Running autoheader"
	$DRYRUN autoheader
      fi
      echo "$program: Running automake $am_opt"
      $DRYRUN automake --add-missing $am_opt
      echo "$program: Running autoconf"
      $DRYRUN autoconf
    )
  fi
done

conf_flags=

echo "$program: Running ./configure $conf_flags $@"
if test "X$DRYRUN" = X; then
  $DRYRUN ./configure $conf_flags "$@" \
  && echo "$program: Now type \`make' to compile $PACKAGE" || exit 1
else
  $DRYRUN ./configure $conf_flags "$@"
fi
