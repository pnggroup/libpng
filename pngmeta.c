/* pngmeta - PNG metadata filter to text, SOIF or HTML

   For libpng 1.0.0 and zlib version 1.1.1 (or later)

   tested up to libpng 1.0.5o and zlib 1.1.3
   
   Copyright 1998-2000 Dave Beckett, ILRT, University of Bristol
   http://purl.org/net/dajobe/


   $Id: pngmeta.c,v 1.12 2001/05/16 14:41:06 cmdjb Exp $
   
   The function png_skip_till_end() is a modified version of
   png_read_end() from libpng 1.0.0 by
   Guy Eric Schalnat, Group 42, Inc.
   Andreas Eric Dilger and Glenn Randers-Pehrson
   as well as many others.  The original copyright message follows.
*/

/* pngread.c - read a PNG file
 *
 * libpng 1.0.0
 * For conditions of distribution and use, see copyright notice in png.h
 * Copyright (c) 1995, 1996 Guy Eric Schalnat, Group 42, Inc.
 * Copyright (c) 1996, 1997 Andreas Dilger
 * Copyright (c) 1998, Glenn Randers-Pehrson
 * March 8, 1998
 *
 * This file contains routines that an application calls directly to
 * read a PNG file or stream.
 */

/* png.h - header file for PNG reference library
 *

 [DJB: edited to just leave COPYRIGHT NOTICE ]

 *
 * COPYRIGHT NOTICE:
 *
 * The PNG Reference Library is supplied "AS IS".  The Contributing Authors
 * and Group 42, Inc. disclaim all warranties, expressed or implied,
 * including, without limitation, the warranties of merchantability and of
 * fitness for any purpose.  The Contributing Authors and Group 42, Inc.
 * assume no liability for direct, indirect, incidental, special, exemplary,
 * or consequential damages, which may result from the use of the PNG
 * Reference Library, even if advised of the possibility of such damage.
 *
 * Permission is hereby granted to use, copy, modify, and distribute this
 * source code, or portions hereof, for any purpose, without fee, subject
 * to the following restrictions:
 * 1. The origin of this source code must not be misrepresented.
 * 2. Altered versions must be plainly marked as such and must not be
 *    misrepresented as being the original source.
 * 3. This Copyright notice may not be removed or altered from any source or
 *    altered source distribution.
 *
 * The Contributing Authors and Group 42, Inc. specifically permit, without
 * fee, and encourage the use of this source code as a component to
 * supporting the PNG file format in commercial products.  If you use this
 * source code in a product, acknowledgment is not required but would be
 * appreciated.
*/

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

/* Needed to access various internal PNG chunk routines */
#define PNG_INTERNAL
#include <png.h>

#ifdef __TURBOC__
#include <mem.h>
#endif

/* defined so I can write to a file on gui/windowing platforms */
/*  #define STDERR stderr  */
#ifdef MSDOS
#define STDERR stdout	/* for DOS */
#else
#define STDERR stderr
#endif


#define XMLRDF_MAGIC_FIELD "XMLRDFDATA"


void png_skip_till_end PNGARG((png_structp png_ptr, png_infop info));
void html_quote_string PNGARG((FILE *fd, const char *string));
void print_init PNGARG((FILE *fd, int output_type, const char *filename, const char *uri, int quiet));
void print_start_image PNGARG((FILE *fd, int output_type, const char *filename, const char *uri, int quiet));
void print_kv PNGARG((FILE *fd, int output_type, const char *field, const char *value));
void print_end_image PNGARG((FILE *fd, int output_type));
void print_finish PNGARG((FILE *fd, int output_type));
void user_warning_fn PNGARG((png_structp png_ptr, png_const_charp warning_msg));


const char *progname;


/* static - used to store magic RDF field if seen */
static char *xmlrdf_magic_block=NULL;


/* read data, ignoring IDATs, till the end of the png file.
   
   Will not read past the end of the file, will verify the end is
   accurate, and will read any comments or time information at the
   end of the file, if info is not NULL. */
void
png_skip_till_end(png_structp png_ptr, png_infop info_ptr)
{
   png_byte chunk_length[4];
   png_uint_32 length;

   length=png_ptr->idat_size;
   
  /* Skip IDAT chunks */
   do
   {
     png_crc_finish(png_ptr, length);

     png_read_data(png_ptr, chunk_length, 4);
     length = png_get_uint_32(chunk_length);

     png_reset_crc(png_ptr);
     png_crc_read(png_ptr, png_ptr->chunk_name, 4);
   } while (!png_memcmp(png_ptr->chunk_name, png_IDAT, 4));
   
   png_ptr->mode |= PNG_AFTER_IDAT;
   
   do
   {
     if (!png_memcmp(png_ptr->chunk_name, png_IHDR, 4))
       png_handle_IHDR(png_ptr, info_ptr, length);
     else if (!png_memcmp(png_ptr->chunk_name, png_IDAT, 4))
     {
       /* Zero length IDATs are legal after the last IDAT has been
        * read, but not after other chunks have been read.
        */
       if (length > 0 || png_ptr->mode & PNG_AFTER_IDAT)
         png_error(png_ptr, "Too many IDAT's found");
       else
         png_crc_finish(png_ptr, 0);
     }
#if defined(PNG_READ_tIME_SUPPORTED)
     else if (!png_memcmp(png_ptr->chunk_name, png_tIME, 4))
       png_handle_tIME(png_ptr, info_ptr, length);
#endif
#if defined(PNG_READ_tEXt_SUPPORTED)
     else if (!png_memcmp(png_ptr->chunk_name, png_tEXt, 4))
       png_handle_tEXt(png_ptr, info_ptr, length);
#endif
#if defined(PNG_READ_zTXt_SUPPORTED)
     else if (!png_memcmp(png_ptr->chunk_name, png_zTXt, 4))
       png_handle_zTXt(png_ptr, info_ptr, length);
#endif
#if defined(PNG_READ_iTXt_SUPPORTED)
      else if (!png_memcmp(png_ptr->chunk_name, png_iTXt, 4))
         png_handle_iTXt(png_ptr, info_ptr, length);
#endif
     else if (!png_memcmp(png_ptr->chunk_name, png_IEND, 4))
       png_handle_IEND(png_ptr, info_ptr, length);
     else
       png_handle_unknown(png_ptr, info_ptr, length);

     if (!(png_ptr->mode & PNG_HAVE_IEND)) {
       png_read_data(png_ptr, chunk_length, 4);
       length = png_get_uint_32(chunk_length);
     
       png_reset_crc(png_ptr);
       png_crc_read(png_ptr, png_ptr->chunk_name, 4);
     }
     
   } while (!(png_ptr->mode & PNG_HAVE_IEND));
}


#define OUTPUT_TEXT 0
#define OUTPUT_SOIF 1
#define OUTPUT_HTML 2
#define OUTPUT_XRDF 3


void html_quote_string(FILE *fd, const char *string)
{
  char c;
  char const * p=string;
  while ((c = *p++)) {
    if (c == '&')
      fputs("&amp;", fd);
    else if (c== '<')
      fputs("&lt;", fd);
    else if (c== '>')
      fputs("&gt;", fd);
    else
      fputc(c, fd);
  }
}


void print_init(FILE *fd, int output_type, const char *filename,
                const char *uri, int quiet)
{
  switch (output_type)
    {
      case OUTPUT_SOIF:
	break;
      
      case OUTPUT_HTML:
	fputs("<!DOCTYPE html PUBLIC '-//W3C//DTD XHTML 1.0 Transitional//EN' 'blah'>\n", fd);
	fputs("<html>\n<head>\n<title>Metadata for ", fd);
	html_quote_string(fd, filename); 
	fputs("</title>\n</head>\n<body>\n\n", fd);
	break;
      
      case OUTPUT_XRDF:
        fputs("<rdf:RDF xmlns:rdf='http://www.w3.org/1999/02/22-rdf-syntax-ns#'\n", fd);
        fputs("         xmlns:png='http://www.w3.org/2000/08/pngmeta#'\n", fd);
        fputs("         xmlns:dc='http://purl.org/dc/elements/1.0/'>\n", fd);
        break;

      default: /* OUTPUT_TEXT */
        if (!quiet)
          fprintf(fd, "%s: PNG metadata for %s:\n", progname, filename);
    }
}


void print_start_image(FILE *fd, int output_type, const char *filename,
                       const char *uri, int quiet)
{
  switch (output_type)
    {
      case OUTPUT_SOIF:
        if (uri)
          fprintf(fd, "@FILE { %s\n", uri);
        else
          fprintf(fd, "@FILE { %s\n", filename);
	break;
      
      case OUTPUT_HTML:
	fputs("<h1>Metadata for ", fd);
	html_quote_string(fd, filename);
	fputs("</h1>\n<dl>\n", fd);
	break;
      
      case OUTPUT_XRDF:
        if (uri)
          fprintf(fd, "  <png:Image about=\"%s\"\n", uri);
        else {
          fputs("  <png:Image about=\"", fd);
          html_quote_string(fd, filename); 
          fputs("\">\n", fd);
        }
        fputs("    <dc:type>image/png</dc:type>\n", fd);

        break;

      default: /* OUTPUT_TEXT */
        break;
    }
}


void print_kv(FILE *fd, int output_type, const char *field, const char *value) 
{
  if (*value)
    switch (output_type)
      {
	case OUTPUT_SOIF:
	  /* SOIF: "KEY{SIZE}:\tVALUE\n" */
	  fprintf(fd, "%s{%d}:\t%s\n", field, (int)strlen(value), value);
	  break;
	
	case OUTPUT_HTML:
	  /* HTML: <dt>field<br /></dt>\n<dd>value</dd>\n" */
	  fputs("  <dt>", fd);
	  html_quote_string(fd, field);
	  fputs("<br /></dt>\n  <dd>", fd);
	  html_quote_string(fd, value);
	  fputs("</dd>\n\n", fd);
	  break;
	
        case OUTPUT_XRDF:
          /* RDF: simple flat text properties  */


          if(strstr(field, XMLRDF_MAGIC_FIELD)) {
            xmlrdf_magic_block=(char*)value;
            break;
          }

          /* start tag */
          fputs("    <png:", fd);
          html_quote_string(fd, field);
          fputs(">", fd);

          /* If value starts with <RDF, assume it is RDF and don't
             HTML escape it */
          if (!strncasecmp(value, "<RDF", 4))
            fputs(value, fd);
          else
            html_quote_string(fd, value);

          /* end tag */
          fputs("</png:", fd);
          html_quote_string(fd, field);
          fputs(">\n", fd);
          break;

	default: /* OUTPUT_TEXT */
	  fprintf(fd, "%s: %s\n", field, value);
      }
}


void
print_end_image(FILE *fd, int output_type)
{
  switch (output_type)
    {
      case OUTPUT_SOIF:
	break;
      
      case OUTPUT_HTML:
	fputs("</dl>\n\n", fd);
	break;
      
      case OUTPUT_XRDF:
        fputs("  </png:Image>\n", fd);
        break;

      default: /* OUTPUT_TEXT */
        break;
    }
}


void print_finish(FILE *fd, int output_type)
{
  switch (output_type)
    {
      case OUTPUT_SOIF:
	fputs("}\n", fd);
	break;
      
      case OUTPUT_HTML:
	fprintf(fd, "<hr /><small>Created by %s V%s</small>\n\n</body>\n</html>\n", progname, VERSION);
	break;
	
      case OUTPUT_XRDF:
        fprintf(fd, "</rdf:RDF>\n\n<!--Created by %s V%s -->\n", progname, VERSION);
        break;
        
      /* case OUTPUT_TEXT / default */
      /* do nothing */
    }
}


static const char *png_color_type[] = {
  "Grayscale",
  "Undefined type",
  "RGB",
  "Palette",
  "Grayscale with Alpha",
  "Undefined type",
  "RGB with Alpha"
};


/* Throw away warnings (die on errors as usual) */
void user_warning_fn (png_structp png_ptr, png_const_charp warning_msg)
{

}



/* MAIN BODY */
int main(int argc, char *argv[])
{
  png_structp png_ptr;
  png_infop info_ptr;
  png_infop end_info;
  FILE *in_fp = stdin;
  FILE *out_fp = stdout;
  const char *pngfile = "stdin";
  int output_type = OUTPUT_TEXT;
  int quiet = 0;
  int all = 0;
  int usingfile = 0;
  int usage = 0;
  int help = 0;
  int version = 0;
  int i;
  char *p;
  char *uri= NULL;
#ifdef HAVE_PNG_GET_TEXT
  png_textp text_ptr;
  int num_text = 0;
#endif

  /* Make progname just become the program name, not the full path -
     this is file system type specific since / is used as the
     separator */
  progname = *argv++; argc--;
  if((p=strrchr(progname, '/'))) 
    progname=p+1;
  
  
  /* Automagically output SOIF when this program is called PngImage.sum */
  if(!strcmp(progname, "PngImage.sum"))
    output_type = OUTPUT_SOIF;    

  /* Automagically output RDF when this program is called PngImage.rdfsum */
  if(!strcmp(progname, "PngImage.rdfsum"))
    output_type = OUTPUT_XRDF;    

  
  while (*argv) {
    char *arg=argv[0];
    int l=strlen(arg);
    
    if (*arg != '-')
      break;

    if(l < 2) {
      usage=1;
      break;
    }
    
    if (arg[0] == '-' && arg[1] != '-') {
      arg++;
    } else {
      /* found '--' or equivalent */
      if (l==2) {
        argv++;
        argc--;
        break;
      }
      arg+=2;
    }
    

    if (!strcmp(arg, "soif")) {
      output_type = OUTPUT_SOIF;
    } else if (!strcmp(arg, "html")) {
      output_type = OUTPUT_HTML;
    } else if (!strcmp(arg, "xrdf")) {
      output_type = OUTPUT_XRDF;
    } else if (!strcmp(arg, "quiet")) {
      quiet = 1;
    } else if (!strcmp(arg, "all")) {
      all = 1;
    } else if (!strcmp(arg, "help")) {
      help = 1;
      break;
    } else if (!strcmp(arg, "version")) {
      version = 1;
      break;
    } else if (!strcmp(arg, "uri")) {
      argv++;
      argc--;
      if (!argc) {
        fprintf(STDERR, "%s: option --uri requires an argument\n", progname);
        usage=1;
      } else {
        uri=*argv;
      }
    } else {
      fprintf(STDERR, "%s: invalid option -- %s\n", progname, arg);
      usage = 1;
      break;
    }
    argv++;
    argc--;
  }

  if (!usage && !help && !version) {
    if (!argc)
      /* nop */;
    else if (argc == 1) {
      pngfile = *argv;
      if (!(in_fp = fopen (pngfile, "rb"))) {
	fprintf(STDERR, "%s: Could not open input file %s - ", progname, pngfile);
	fflush(STDERR);
	perror(NULL);
	exit(1);
      }
      usingfile = 1;
    } else {
      usage = 1;
    }
  } 


  if(usage) {
    fprintf(STDERR, "Try `%s --help' for more information\n", progname);
    exit(1);
  }
  
  
  if (help || version) {
    fprintf(STDERR, "%s %s (built with libpng %s and zlib %s)\n", progname, VERSION, PNG_LIBPNG_VER_STRING, ZLIB_VERSION);
    if (help) {
      fprintf(STDERR, "USAGE: %s [OPTIONS]... FILE\n", progname);
      fprintf(STDERR, "Display metadata information from a PNG image in FILE\n\n");
      fprintf(STDERR, "  --all          output information about image size etc.\n");
      fprintf(STDERR, "  --html         format output in HTML format\n");
      fprintf(STDERR, "  --quiet        suppress output of banner\n");
      fprintf(STDERR, "  --soif         format output in SOIF format\n");
      fprintf(STDERR, "  --uri URI      set the URI for SOIF and XML/RDF formats\n");
      fprintf(STDERR, "  --xrdf         format output in XML/RDF format\n");
      fprintf(STDERR, "  --help         display this help and exit\n");
      fprintf(STDERR, "  --version      output version information and exit\n");
      fprintf(STDERR, "\nCopyright 1998-2000 Dave Beckett, ILRT, University of Bristol\nhttp://purl.org/net/dajobe/\n");
    }
    exit(1);
  }

  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                                   (void*)NULL, NULL, user_warning_fn);
  if (!png_ptr) {
    fprintf(STDERR, "%s: libpng failed to create read structure\n", progname);
    exit(1);
  }

  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr)
  {
    png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
    fprintf(STDERR, "%s: libpng failed to create info structure\n", progname);
    exit(1);
  }

  /* This is necessary to zero text pointers */
  end_info = png_create_info_struct(png_ptr);
  if (!end_info)
  {
    png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
    fprintf(STDERR, "%s: libpng failed to create end info structure\n", progname);
    exit(1);
  }
  

  if (setjmp(png_ptr->jmpbuf))
  {
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    fprintf(STDERR, "%s: libpng read error for %s\n", progname, pngfile);
    fclose(in_fp);
    exit(1);
  }
  

  /* Initialise data input */
  png_init_io(png_ptr, in_fp);
  png_read_info(png_ptr, info_ptr);
  
  print_init(out_fp, output_type, pngfile, uri, quiet);

  print_start_image(out_fp, output_type, pngfile, uri, quiet);
  
  if (output_type == OUTPUT_SOIF || all) {
/*
  Harvest V1.4 GIF summarizer outputs this:
    
  image-format: GIF87|GIF89
  image-colors: [bits-per-pixel]
  Comment: [GIF Comment Extension text]
  image-width: [w]
  image-height: [h]
  image-type: interlaced|non-interlaced
  thumbnail-data: [data for 64x64 GIF image]
  
  The latter is done via:
    giftopnm giffile | pnmscale -width 64 -height 64 | ppmquant 256 | ppmtogif
    
*/
    char value[80]; /* Sorry for the fixed-size buffer (big enough) */
    
    print_kv(out_fp, output_type, "image-format", "PNG");
    
    sprintf(value, "%d", info_ptr->bit_depth);
    print_kv(out_fp, output_type, "image-colors", value);
    
    sprintf(value, "%ld", info_ptr->width);
    print_kv(out_fp, output_type, "image-width", value);
    
    sprintf(value, "%ld", info_ptr->height);
    print_kv(out_fp, output_type, "image-height", value);
    
    sprintf(value, "%s, %sinterlaced",
	    (info_ptr->color_type>6) ? png_color_type[1] : png_color_type[info_ptr->color_type],
	    info_ptr->interlace_type ? "" : "non-");

    print_kv(out_fp, output_type, "image-type", value);
  }
  
  /* Local function */
  png_skip_till_end(png_ptr, end_info);


#ifdef HAVE_PNG_GET_TEXT
  if(png_get_text(png_ptr, info_ptr, &text_ptr, &num_text) > 0) {
    for (i = 0; i < num_text; i++)
      print_kv(out_fp, output_type, text_ptr[i].key, text_ptr[i].text);
  }
#else
  /* Print text keywords before IDAT */
  for (i = 0; i < info_ptr->num_text; i++)
    print_kv(out_fp, output_type, info_ptr->text[i].key, info_ptr->text[i].text);
  
  /* Print text keywords after IDAT */
  for (i = 0; i < end_info->num_text; i++)
    print_kv(out_fp, output_type, end_info->text[i].key, end_info->text[i].text);
    
#endif
  
  /* Print modification time (tIME chunk) if present */
  if (info_ptr->valid & PNG_INFO_tIME)
    print_kv(out_fp, output_type, "Modification Time",
             png_convert_to_rfc1123(png_ptr, &info_ptr->mod_time));
  else if (end_info->valid & PNG_INFO_tIME)
    print_kv(out_fp, output_type, "Modification Time",
             png_convert_to_rfc1123(png_ptr, &end_info->mod_time));


  print_end_image(out_fp, output_type);

  /* print rest of RDF/XML stuff */
  if(xmlrdf_magic_block && output_type == OUTPUT_XRDF) {
    fputs(xmlrdf_magic_block, out_fp);
  }

  print_finish(out_fp, output_type);
  
  
  /* Cleanup */
  png_read_destroy(png_ptr, info_ptr, end_info);

  fclose(in_fp);
  
  exit (0);
}
