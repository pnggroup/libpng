readme.txt - for libpng 0.8

This is the second beta version of libpng 1.0.  I've updated most
of the stuff I want to before the final 1.0 version.  Remaining
to do are the medium memory model support (which I'll put in
as soon as we test this version), better dithering, and any bug
fixes and makefile/include additions.  I expect a third (and
perhaps final) beta after zlib is officially 1.0.

I've tried to incorporate all the changes and makefiles everyone
sent me.  However, I may of lost some in the flood.  If you sent
me a change and I didn't put it in, send it again.  Sorry.

Updates from libpng 0.71 include a new transformation,
png_set_filler(), which replaces png_set_rgbx() and
png_set_xrgb().  The old functions will be supported for
awhile, but I'd suggest changing to the new function.  Also,
I've added defines in png.h to remove unwanted code from the
compiled library.  I've added a new field to png_realloc(), and
fixed various bugs.  I've also split up pngstub.c into pngmem.c,
pngio.c, and pngerror.c, in case you need to just change some of
these.  I've pulled pngconf.h out of png.h, so you don't have to
remake your changes every new release.  I've added a function to
update the png_info structure after you're done setting up your
transformations (png_read_update_info()).  The complete list of
changes is in pngchang.txt.  Most of you won't be much affected
by any of this.  Some of you will want to use the new features.

For a detailed description on using libpng, read libpng.txt.  For
usage information and restrictions (what little they are) on libpng,
see png.h.  For a description on using zlib (the compression library
used by libpng) and zlib's restrictions, see zlib.h

I have included a general makefile, but you may have to modify it
for your own needs.

You will need zlib 0.95 or later to run this.  zlib is a compression
library that is useful for more things then just png files.  If
you need a compression library, check out zlib.h

zlib should be available at the same place that libpng is.
If not, it should be at ftp.uu.net in /graphics/png
Eventually, it will be at ftp.uu.net in /pub/archiving/zip/zlib

You will also want a copy of the PNG specification.  It should
be available at the same place you picked up libpng.  If it is
not there, try ftp.uu.net in the /graphics/png directory.

This code is currently being archived at ftp.uu.net in the
/graphics/png directory, and at ftp.group42.com (204.94.158.25)
in the /pub/png directory, and on CompuServe, Lib 20 (PNG SUPPORT)
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

You can reach me at:

internet: schalnat@group42.com
CompuServe: 75501,1625

Please do not send me general questions about PNG.  Send them to
the address in the specification.  At the same time, please do
not send libpng questions to that address, send them to me.  I'll
get them in the end anyway.  If you have a question about something
in the PNG specification that is related to using libpng, send it
to me.  Send me any questions that start with "I was using libpng,
and ...".  If in doubt, send questions to me.  I'll bounce them
to others, if necessary.

Please do not send suggestions on how to change PNG.  We have
been discussing PNG for 9 months now, and it is official and
finished.  If you have suggestions for libpng, however, I'll
gladly listen.  Even if your suggestion is not used for version
1.0, it may be used later.

Good luck, and happy coding.

-Guy Eric Schalnat
 Group 42, Inc.
 Internet: schalnat@group42.com
 CompuServe: 75501,1625
 Web: www.group42.com
 FTP: ftp.group42.com (204.94.158.25)

