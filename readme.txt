readme.txt - for libpng 0.71

This is the first beta version of libpng 1.0.  By beta, I mean that
all the code for 1.0 is there, and it works on all the machines
I have running all the tests I have devised.  However, there is
always one more bug (at least), and I don't have many #define's in
the code (yet) for various platforms that I do not have.  Also, I'd
like to see if I can get the code to compile with as few warnings
as possible.  Finally, as people use my code, they may have
suggestions for additions that will make pnglib easier to port.

For a detailed description on using libpng, read libpng.txt.  For
usage information and restrictions (what little they are) on libpng,
see png.h.  For a description on using zlib (the compression library
used by libpng) and zlib's restrictions, see zlib.h

I have included a make file, but you will probably have to modify it
for your own needs.  I'm using Borland C++, running large memory
model on Windows 3.11, but it should work on almost anything.  Support
for medium memory model is planned, but is not in 1.0 (probably in 1.1).

You will need zlib 0.93 to run this.  zlib is a compression
library that is useful for more things then just png files.  If
you need a compression library, check out zlib.h

zlib should be available at the same place that libpng is.
If not, it should be at ftp.uu.net in /graphics/png
Eventually, it will be at ftp.uu.net in /pub/archiving/zip/zlib

You will also want a copy of the PNG specification.  It should
be available at the same place you picked up libpng.  If it is
not there, try ftp.uu.net in the /graphics/png directory.

This code is currently being archived at ftp.uu.net in the
/graphics/png directory, and at ftp.group42.com in the /pub/png
directory, and on CompuServe, Lib 20 (PNG) at GO GRAPHSUP.
If you can't find it in any of those places, e-mail me, and I'll
tell you where it is.

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

internet: schalnat&group42.com
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
been discussing PNG for 6 months now, and it is official and
finished.  If you have suggestions for libpng, however, I'll
gladly listen.  Even if your suggestion is not used for version
1.0, it may be used later.

Good luck, and happy coding.

-Guy Eric Schalnat
 Group 42, Inc.
 Internet: schalnat@group42.com
 CompuServe: 75501,1625
 Web: www.group42.com
 FTP: ftp.group42.com

