/*
 png2uri

 NO COPYRIGHT RIGHTS ARE CLAIMED TO THIS SOFTWARE.

 To the extent possible under law, the author has waived all copyright and
 related or neighboring rights to this work. This work is published from
 the United States of America in 2012.

 The png2uri software may be used freely in any way.

 The source is the original work of the person named below.  No other person
 or organization has made contributions to this work.

 ORIGINAL AUTHORS
     The following people have contributed to the code or comments in this
     file.  None of the people below claim any rights with regard to the
     contents of this file.

     Glenn Randers-Pehrson <glennrp@users.sourceforge.net>

 png2uri is a command-line application that creates an HTML "img" tag on
 standard output containing a data URI, from a PNG file or from standard
 input. Other formats besides the default, PNG, are supported, via the "-f"
 or "--format" option or, if that option was not supplied, by inspection of
 the filename extension.

 Usage: png2uri [-f format|--format format] [-u|--uri_only] [file]

        options: -f|--format TYPE: 
                 write "image/TYPE" instead of "image/png" in the
                 data uri.  TYPE can be png, jpg, jpeg, bmp, or gif,
                 or any of those in upper case.  To write any other
                 type, include the complete MIME type as in
                 "--format image/x-jng" or "-f audio/mpeg".

                 -u|--uri_only|--url_only
                 omit the surrounding "img" tag and only write the
                 data URI.

 Requires /bin/sh and a base64(1) that encodes its input in base64
 according to RFC-3548 and writes the result on standard output, and
 a working "sed".  Versions 1.0.0 through 1.0.3 used uuencode(1) instead
 of base64(), but a surprising number of machines that I've tried
 (5 out of 11) don't have uuencode installed, while all 11 had base64(1)
 which is a "core utility" rather than a "shar utility".  Using base64()
 has an additional advantage that the lines are always folded to the same
 width, 76 characters wide, while uuencode sometimes writes shorter lines.

 REFERENCES:
    http://en.wikipedia.org/wiki/Data_URI_scheme

    Masinter, L. "RFC 2397 - The "data" URL scheme",
      Internet Engineering Task Force, August 1998

    http://en.wikipedia.org/wiki/Base64

    IETF, "RFC 3548 - The Base16, Base32, and Base64 Data Encodings",
      July 2003.

 OTHER IMPLEMENTATIONS

 If you prefer a web-based converter or a java application, this isn't
 it. Use your search engine and look for "png data uri" to find one.
 Here are some:

   A PHP script:
      http://css-tricks.com/snippets/php/create-data-uris/

   Hixie's web-based converter
      http://software.hixie.ch/utilities/cgi/data/data/
 
   Websemantics' web-based converter
      http://websemantics.co.uk/online_tools/image_to_data_uri_convertor

   Mike Scalora's web-based converter
      http://www.scalora.org/projects/uriencoder/

   Data URL Maker (a web-based drag-and-drop converter), CSS Optimizer,
   and Data URL Toolkit (a PERL implementation)
      http://dataurl.net/#about

 CHANGE LOG

   Version 1.0.0, July 3, 2012:

      Initial release.

   Version 1.0.1, July 4, 2012:

      The string "base64" could conceivably appear within the encoded data
      and any line containing "base64" is removed; revised script to
      use "====" instead to remove the first line of uuencode output.

      Implemented "-u" and "--url_only" option.

   Version 1.0.2, July 4, 2012:

      Implemented "-f TYPE" and "--format TYPE" option.

   Version 1.0.3, July 5, 2012:

      Changed environment variable "format" to PNG2URI_FMT reduce possibility`
      of name conflict, and changed "uri_only" to PNG2URI_URI.  Fixed some
      indentation.

   Version 1.0.4, July 6, 2012:

      Use "base64" instead of "uuencode" when "uname" returns "Linux" to
      generate the output, because "base64" seems to be more prevalently
      available. On Gnu/Linux platforms, uuencode must be installed as part
      of "sharutils", which often doesnn't happen.  SunOS and FreeBSD don't
      supply "base64" so we use "uuencode" instead.

 TO DO

 1. Test on various other platforms. I assume we'll soon find one that has
    neither "uuencode" nor "base64".  The following have been tried

    Ubuntu 10.04 (uuencode output was folded to 60 characters)
       Linux glenn.rp 2.6.32-41-generic #91-Ubuntu SMP Wed Jun 13 11:43:55
          UTC 2012 x86_64 GNU/Linux
       Linux nancy.rp 2.6.32-41-generic #91-Ubuntu SMP Wed Jun 13 11:43:55
          UTC 2012 x86_64 GNU/Linux (uuencode not found.  Fixed that with
          "sudo aptitude install sharutils", then finally fixed by using
          base64() instead of uuencode())

    Ubuntu 12.04 (uuencode not found but manpage for POSIX uuencode exists;
       base64 works)
       Linux scooby 3.0.0-22-generic #36-Ubuntu SMP Tue Jun 12 17:37:42 UTC 2012
          x86_64 x86_64 x86_64 GNU/Linux

    SunOS 5.10 (uuencode data is folded to 68 characters)
       Does not work with "base64"!  Reconfigured png2uri to use "uuencode".
       on non-Linux platforms.
       SunOS freddy 5.10 Generic_147441-13 i86pc i386 i86pc
       SunOS blade 5.10 Generic_144488-12 sun4u sparc SUNW,Sun-Blade-2500

    Sunos 5.11 (uuencode data is folded to 68 characters)
       SunOS weerd 5.11 oi_151a4 i86pc i386 i86pc

    FreeBSD 8.0 (uuencode data is folded to 76 characters)
       "base64 not found"; use uuencode.
       FreeBSD shaggy.simplesystems.org 8.0-RELEASE-p1 FreeBSD 8.0-RELEASE-p1
           #1: Mon Dec 21 11:26:14 CST 2009
           zsh@shaggy.simplesystems.org:/usr/src/sys/i386/compile/GENERIC
           i386

    Red Hat? (uuencode not found but manpage for POSIX uuencode
    exists; worked when base64 was used instead of uuencode)
       Linux studio.imagemagick.org 2.6.18-308.4.1.el5xen #1 SMP Tue Apr 17
           17:49:15 EDT 2012 x86_64 x86_64 x86_64 GNU/Linux
       Linux magick.imagemagick.org 3.4.4-3.fc17.x86_64 #1 SMP Tue Jun 26
           20:54:56 UTC 2012 x86_64 x86_64 x86_64 GNU/Linux
       Two other Redhat Enterprise platforms also worked with base64, but
           uuencode was not installed.

 2. Find out if the script works on Windows and Mac or can be modified to
 do so.

 3. Make it work as a drag-and-drop application.

 4. But keep it simple! 

*/
