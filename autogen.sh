#! /bin/sh
#
# Run 'autoreconf' to build 'configure', 'Makefile.in' and other configure
# control files.
args=
force=
init=
while test $# -gt 0
do
   case "$1" in
      -f|--force)
         force=1
         args="$args $1";;

      -V|--version)
         sed -n -e \
            '/PNG_HEADER_VERSION_STRING/,+1s/^[^"]*" \([^"]*\).."[^"]*$/\1/p' \
            png.h
         exit 0;;

      --init)
         init=1;;

      -h|--help)
         echo "$0: run autoreconf to update configure files" >&2
         echo " options:" >&2
         echo "  --version: print the version of this libpng" >&2
         echo "  --force: ignore date stamps and make all files" >&2
         echo "  --init: do a complete re-initialization" >&2
         echo "  others: passed to autoreconf, use autoreconf --help" >&2
         exit 1;;

      *)
         args="$args $1";;
   esac

   shift
done

if test -r configure -a -z "$init"
then
   # Configure exists, either an update or a tarball distribution
   if test ! -d .git -a -z "$force"
   then
      echo "autogen: running with --force to update a non-GIT distribution" >&2
      args="--force $args"
   fi
   autoreconf --warnings=all $args
else
   # No configure: assume this is a clean tree and everything needs to be
   # generated
   # autoupdate
   autoreconf --warnings=all --force --install $args
fi
