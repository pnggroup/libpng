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
 input.

 Usage: png2uri [-u|--uri_only] [file]

        options: -u|--uri_only|--url_only: omit the surrounding "img" tag
                 and only write the data URI.

 Requires /bin/sh and a uuencode(1) that takes the "-m" option to mean
 to encode in base64.  A surprising number of machines that I've tried
 (3 out of 9) don't have uuencode installed.

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

 CHANGE LOG

   Version 1.0.0, July 3, 2012:

      Initial release.

   Version 1.0.1, July 4, 2012:

      The string "base64" could conceivably appear within the encoded data
      and any line containing "base64" is removed; revised script to
      use "====" instead to remove the first line of uuencode output.

      Implemented "-u" and "--url_only" option.

 TO DO

 1. Test on various platforms. The following have been tried

    Ubuntu 10.04 (data is folded to 60 characters)
       Linux glenn.rp 2.6.32-41-generic #91-Ubuntu SMP Wed Jun 13 11:43:55
          UTC 2012 x86_64 GNU/Linux
       Linux nancy.rp 2.6.32-41-generic #91-Ubuntu SMP Wed Jun 13 11:43:55
          UTC 2012 x86_64 GNU/Linux (uuencode not found.  Fixed that with
          "sudo aptitude install sharutils", then data folded to 60 chars)

    Ubuntu 12.04 (FAILED: uuencode not found but manpage for POSIX uuencode
    exists)
       Linux scooby 3.0.0-22-generic #36-Ubuntu SMP Tue Jun 12 17:37:42 UTC 2012
          x86_64 x86_64 x86_64 GNU/Linux

    SunOS 5.10 (data is folded to 68 characters)
       SunOS freddy 5.10 Generic_147441-13 i86pc i386 i86pc
       SunOS blade 5.10 Generic_144488-12 sun4u sparc SUNW,Sun-Blade-2500

    Sunos 5.11 (data is folded to 68 characters)
       SunOS weerd 5.11 oi_151a4 i86pc i386 i86pc

    FreeBSD 8.0 (data is folded to 76 characters)
       FreeBSD shaggy.simplesystems.org 8.0-RELEASE-p1 FreeBSD 8.0-RELEASE-p1
           #1: Mon Dec 21 11:26:14 CST 2009
           zsh@shaggy.simplesystems.org:/usr/src/sys/i386/compile/GENERIC
           i386

    Red Hat? (FAILED: uuencode not found but manpage for POSIX uuencode
    exists)
       Linux studio.imagemagick.org 2.6.18-308.4.1.el5xen #1 SMP Tue Apr 17
           17:49:15 EDT 2012 x86_64 x86_64 x86_64 GNU/Linux
       Linux magick.imagemagick.org 3.4.4-3.fc17.x86_64 #1 SMP Tue Jun 26
           20:54:56 UTC 2012 x86_64 x86_64 x86_64 GNU/Linux

 2. This script can be trivially modified to support another image format
 (e.g., change PNG to JPG and "image/png" to "image/jpeg" throughout).
 To do: do that, via a "-f/--format jpg|jpeg|png|bmp|gif" option and by
 inspecting the filename extension.

 3. Find out if the script works on Windows and Mac or can be modified to
 do so.

 4. Make it work as a drag-and-drop application.

 5. But keep it simple!

*/
