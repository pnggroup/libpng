readme.txt - for libpng 0.89

This is a bug fix for the third beta version of libpng 1.0.  The
changes from libpng-0.88 are bug fixes and some changes to the
API itself to increase robustness with shared libraries.  This
release is based on libpng-0.88, but has been modified from that
version by Andreas Dilger <adilger@enel.ucalgary.ca> because the
original author, Guy Schalnat, has not been able to keep up with
the time demands of maintaining this library.

The callback functions for the error/warning messages have changed
since the last release because their implementation was broken,
and it was thought best to change the API itself (which was only
introduced in libpng-0.88 itself) to alert the user to the change,
rather than mislead the user into thinking their application was
OK after re-compiling.  This means that calls to png_set_message_fn()
no longer exist, because the previously suggested method of calling
them before png_read_init() or png_write_init() is now ineffective.

The preferred method of setting the error and warning callbacks
has been incorporated into the allocation of the png_struct and
info_struct itself, which allow them to be safely used during the
initialization of the structure, as well as to keep the size of
the png_struct internal to the library, rather than at compile time
of the application.  This will hopefully remove any problems with
dynamically linked libraries, and should be considered the preferred
method of creating these structures, although the previous
initialization API is still available for compatibility.  See libpng.txt
for more information on the new API.

The changes made to the library, and bugs fixed are based on discussions
on the PNG implementation mailing list <png-implement@dworking.wustl.edu>
and not on material submitted to Guy.

For a detailed description on using libpng, read libpng.txt.  For
usage information and restrictions (what little they are) on libpng,
see png.h.  For a description on using zlib (the compression library
used by libpng) and zlib's restrictions, see zlib.h

I have included a general makefile, as well as several machine and compiler
specific ones, but you may have to modify one for your own needs.

You will need zlib 0.95 or later to run this.  zlib is a compression
library that is useful for more things then just png files.  If
you need a compression library, check out zlib.h.  There was a bug in
zlib <= 0.99 which caused it to generate invalid compression streams
on some occasions.  Later versions of zlib do not have this problem.

zlib should be available at the same place that libpng is.
If not, it should be at ftp.uu.net in /graphics/png
Eventually, it will be at ftp.uu.net in /pub/archiving/zip/zlib

You may also want a copy of the PNG specification.  It should
be available at the same place you picked up libpng.  If it is
not there, try ftp.uu.net in the /graphics/png directory.

This code is currently being archived at ftp.uu.net in the
/graphics/png directory, and on CompuServe, Lib 20 (PNG SUPPORT)
at GO GRAPHSUP.  If you can't find it in any of those places,
e-mail me, and I'll help you find it.

If you have any code changes, requests, problems, etc., please e-mail
them to me.  Also, I'd appreciate any make files or project files,
and any modifications you needed to make to get libpng to compile,
along with a #define variable to tell what compiler/system you are on.
If you needed to add transformations to libpng, or wish libpng would
provide the image in a different way, drop me a note (and code, if
possible), so I can consider supporting the transformation.
Finally, if you get any warning messages when compiling libpng
(note: not zlib), and they are easy to fix, I'd appreciate the
fix.  Please mention "libpng" somewhere in the subject line.  Thanks.

This release was created and will be supported by myself, and the
PNG group.

adilger@enel.ucalgary.ca
png-implement@dworkin.wustl.edu

You can reach Guy, the original libpng author, at (internet preferred):

internet: schalnat@group42.com
CompuServe: 75501,1625

Please do not send general questions about PNG.  Send them to
the address in the specification.  At the same time, please do
not send libpng questions to that address, send them to me.  I'll
get them in the end anyway.  If you have a question about something
in the PNG specification that is related to using libpng, send it
to me.  Send me any questions that start with "I was using libpng,
and ...".  If in doubt, send questions to me.  I'll bounce them
to others, if necessary.

Please do not send suggestions on how to change PNG.  We have
been discussing PNG for over a year now, and it is official and
finished.  If you have suggestions for libpng, however, I'll
gladly listen.  Even if your suggestion is not used for version
1.0, it may be used later.

Good luck, and happy coding.

-Guy Eric Schalnat
 Group 42, Inc.
 Internet: schalnat@group42.com
 CompuServe: 75501,1625
 Web: www.group42.com

