#!/bin/awk -f
# scripts/dfn.awk - process a .dfn file
#
# last changed in libpng version 1.5.14 - February 4, 2013
#
# Copyright (c) 2013-2013 Glenn Randers-Pehrson
#
# This code is released under the libpng license.
# For conditions of distribution and use, see the disclaimer
# and license in png.h

# The output of this script is written to the file given by
# the variable 'out', which should be set on the command line.
# Error messages are printed to stdout and if any are printed
# the script will exit with error code 1.

BEGIN{
   out="/dev/null"       # as a flag
   out_count=0           # count of output lines
   err=0                 # set if an error occured
   sort=0                # sort the output
   array[""]=""
}

# The output file must be specified before any input:
NR==1 && out == "/dev/null" {
   print "out=output.file must be given on the command line"
   # but continue without setting the error code, this allows the
   # script to be checked easily
}

# Output can be sorted; two lines are recognized
$1 == "PNG_DFN_START_SORT"{
   sort=0+$2
   next
}

$1 ~ /^PNG_DFN_END_SORT/{
   # Do a very simple, slow, sort; notice that blank lines won't be
   # output by this
   for (entry in array) {
      while (array[entry] != "") {
         key = entry
         value = array[key]
         array[key] = ""

         for (alt in array) {
            if (array[alt] != "" && alt < key) {
               array[key] = value
               value = array[alt]
               key = alt
               array[alt] = ""
            }
         }

         print value >out
      }
   }
   sort=0
   next
}

/^[^"]*PNG_DFN *".*"[^"]*$/{
    # A definition line, apparently correctly formated, extract the
    # definition then replace any doubled "" that remain with a single
    # double quote.  Notice that the original doubled double quotes
    # may have been split by tokenization
    orig=$0

    if (gsub(/^[^"]*PNG_DFN *"/,"") != 1 || gsub(/"[^"]*$/, "") != 1) {
	print "line", NR, "processing failed:"
	print orig
	print $0
	err=1
    } else {
	++out_count
    }

    # Now examine quotes within the value:
    #
    #   @" - delete this and any following spaces
    #   "@ - delete this and any original spaces
    #   @' - replace this by a double quote
    #
    # This allows macro substitution by the C compiler thus:
    #
    #   #define first_name John
    #   #define last_name Smith
    #
    #	PNG_DFN"#define name @'@" first_name "@ @" last_name "@@'"
    #
    # Might get C preprocessed to:
    #
    #   PNG_DFN "#define foo @'@" John "@ @" Smith "@@'"
    #
    # Which this script reduces to:
    #
    #	#define name "John Smith"
    #
    while (sub(/@" */, "")) {
	if (!sub(/ *"@/, "")) {
	    print "unbalanced @\" ... \"@ pair"
	    err=1
	    break
	}
    }

    # Put any needed double quotes in
    gsub(/@'/,"\"")

    # Remove any trailing spaces (not really required, but for
    # editorial consistency
    sub(/ *$/, "")

    if (sort)
       array[$(sort)] = $0

    else
       print $0 >out
    next
}

/PNG_DFN/{
    print "line", NR, "incorrectly formated PNG_DFN line:"
    print $0
    err = 1
}

END{
    if (out_count > 0 || err > 0)
	exit err

    print "no definition lines found"
    exit 1
}
