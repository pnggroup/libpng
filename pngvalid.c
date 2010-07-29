
/* pngvalid.c - validate libpng by constructing then reading png files.
 *
 * Last changed in libpng 1.5.0 [July 5, 2010]
 * Copyright (c) 2010-2010 Glenn Randers-Pehrson
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 *
 * NOTES:
 *   This is a C program that is intended to be linked against libpng.  It
 *   generates bitmaps internally, stores them as PNG files (using the
 *   sequential write code) then reads them back (using the sequential
 *   read code) and validates that the result has the correct data.
 *
 *   The program can be modified and extended to test the correctness of
 *   transformations performed by libpng.
 */

#include "png.h"
#include "zlib.h"   /* For crc32 */

#include <stdlib.h> /* For malloc */
#include <string.h> /* For memcpy, memset */
#include <setjmp.h> /* For jmp_buf, setjmp, longjmp */
#include <math.h>   /* For floor */

/******************************* ERROR UTILITIES ******************************/
static size_t safecat(char *buffer, size_t bufsize, size_t pos, const char *cat)
{
   while (pos < bufsize && cat != NULL && *cat != 0) buffer[pos++] = *cat++;
   if (pos >= bufsize) pos = bufsize-1;
   buffer[pos] = 0;
   return pos;
}

static size_t safecatn(char *buffer, size_t bufsize, size_t pos, int n)
{
   char number[64];
   sprintf(number, "%d", n);
   return safecat(buffer, bufsize, pos, number);
}

static size_t safecatd(char *buffer, size_t bufsize, size_t pos, double d,
   int precision)
{
   char number[64];
   sprintf(number, "%.*f", precision, d);
   return safecat(buffer, bufsize, pos, number);
}

static const char invalid[] = "invalid";
static const char sep[] = ": ";

/* NOTE: this is indexed by ln2(bit_depth)! */
static const char *bit_depths[8] =
{
   "1", "2", "4", "8", "16", invalid, invalid, invalid
};

static const char *colour_types[8] =
{
   "greyscale", invalid, "truecolour", "indexed-colour",
   "greyscale with alpha", invalid, "truecolour with alpha", invalid
};

/* Convenience API to list valid formats: */
static int
next_format(png_bytep colour_type, png_bytep bit_depth)
{
   if (*bit_depth == 0)
   {
      *colour_type = 0, *bit_depth = 1;
      return 1;
   }
   else switch (*colour_type)
   {
   case 0:
      *bit_depth <<= 1;
      if (*bit_depth <= 16) return 1;
      *colour_type = 2;
      *bit_depth = 8;
      return 1;
   case 2:
      *bit_depth <<= 1;
      if (*bit_depth <= 16) return 1;
      *colour_type = 3;
      *bit_depth = 1;
      return 1;
   case 3:
      *bit_depth <<= 1;
      if (*bit_depth <= 8) return 1;
      *colour_type = 4;
      *bit_depth = 8;
      return 1;
   case 4:
      *bit_depth <<= 1;
      if (*bit_depth <= 16) return 1;
      *colour_type = 6;
      *bit_depth = 8;
      return 1;
   case 6:
      *bit_depth <<= 1;
      if (*bit_depth <= 16) return 1;
      break;
   }

   /* Here at the end. */
   return 0;
}

static inline unsigned
sample(png_byte *row, png_byte colour_type, png_byte bit_depth, png_uint_32 x,
   unsigned sample)
{
   png_uint_32 index, result;
   
   /* Find a sample index for the desired sample: */
   x *= bit_depth;
   index = x;
   if ((colour_type & 1) == 0) /* !palette */
   {
      if (colour_type & 2)
         index *= 3, index += sample; /* Colour channels; select one */
      if (colour_type & 4) index += x; /* Alpha channel */
   }

   /* Return the sample from the row as an integer. */
   row += index >> 3;
   result = *row;
   if (bit_depth == 8)
      return result;
   else if (bit_depth > 8)
      return (result << 8) + *++row;
   /* Less than 8 bits per sample. */
   index &= 7;
   return (result >> (8-index-bit_depth)) & ((1U<<bit_depth)-1);
}

/*************************** BASIC PNG FILE WRITING ***************************/
/* A png_sucker takes data from the sequential writer or provides data
 * to the sequential reader.  It can also store the result of a PNG
 * write for later retrieval.
 */
#define SUCKER_BUFFER_SIZE 500 /* arbitrary */
typedef struct png_sucker_buffer
{
   struct png_sucker_buffer* prev;    /* NOTE: stored in reverse order */
   png_byte                  buffer[SUCKER_BUFFER_SIZE];
} png_sucker_buffer;

typedef struct png_sucker_file
{
   struct png_sucker_file* next;      /* as many as you like... */
   char                    name[64];  /* textual name */
   png_uint_32             id;        /* as a convenience to users */
   png_size_t              datacount; /* In this (the last) buffer */
   png_sucker_buffer       data;      /* Last buffer in file */
} png_sucker_file;

#define SUCKER_ERROR 0x345
typedef struct png_sucker
{
   jmp_buf            jmpbuf;
   int                verbose;
   int                nerrors;
   int                nwarnings;
   int                treat_warnings_as_errors;
   char               test[64]; /* Name of test */
   char               error[128];
   /* Read fields */
   png_structp        pread;    /* Used to read a saved file */
   png_infop          piread;
   png_sucker_file*   current;  /* Set when reading */
   png_sucker_buffer* next;     /* Set when reading */
   png_size_t         readpos;  /* Position in *next */
   /* Write fields */
   png_sucker_file*   saved;
   png_structp        pwrite;   /* Used when writing a new file */
   png_infop          piwrite;
   png_size_t         writepos; /* Position in .new */
   char               wname[64];/* Name of file being written */
   png_sucker_buffer  new;      /* The end of the new PNG file being written. */
} png_sucker;

/* Initialization and cleanup */
static void
sucker_init(png_sucker* ps)
{
   memset(ps, 0, sizeof *ps);
   ps->verbose = 0;
   ps->nerrors = ps->nwarnings = 0;
   ps->treat_warnings_as_errors = 0;
   ps->pread = NULL;
   ps->piread = NULL;
   ps->saved = ps->current = NULL;
   ps->next = NULL;
   ps->readpos = 0;
   ps->pwrite = NULL;
   ps->piwrite = NULL;
   ps->writepos = 0;
   ps->new.prev = NULL;
}

static void
sucker_freebuffer(png_sucker_buffer* psb)
{
   if (psb->prev)
   {
      sucker_freebuffer(psb->prev);
      free(psb->prev);
      psb->prev = NULL;
   }
}

static void
sucker_freenew(png_sucker *ps)
{
   sucker_freebuffer(&ps->new);
   ps->writepos = 0;
}

static void
sucker_storenew(png_sucker *ps)
{
   png_sucker_buffer *pb;
   if (ps->writepos != SUCKER_BUFFER_SIZE)
      png_error(ps->pwrite, "invalid store call");
   pb = malloc(sizeof *pb);
   if (pb == NULL)
      png_error(ps->pwrite, "store new: OOM");
   *pb = ps->new;
   ps->new.prev = pb;
   ps->writepos = 0;
}

static void
sucker_freefile(png_sucker_file *pf)
{
   if (pf->next)
      sucker_freefile(pf->next);
   pf->next = NULL;
   sucker_freebuffer(&pf->data);
   pf->datacount = 0;
   free(pf);
}

/* Main interface to file storeage, after writing a new PNG file (see the API
 * below) call sucker_storefile to store the result with the given name and id.
 */
static void
sucker_storefile(png_sucker *ps, png_uint_32 id)
{
   png_sucker_file *pf = malloc(sizeof *pf);
   if (pf == NULL)
      png_error(ps->pwrite, "storefile: OOM");
   safecat(pf->name, sizeof pf->name, 0, ps->wname);
   pf->id = id;
   pf->data = ps->new;
   pf->datacount = ps->writepos;
   ps->new.prev = NULL;
   ps->writepos = 0;

   /* And save it. */
   pf->next = ps->saved;
   ps->saved = pf;
}

/* Generate an error message (in the given buffer) */
static size_t
sucker_message(png_structp pp, char *buffer, size_t bufsize, const char *msg)
{
   size_t pos = 0;
   png_sucker *ps = png_get_error_ptr(pp);

   if (pp == ps->pread)
   {
      /* Reading a file */
      pos = safecat(buffer, bufsize, pos, "read: ");
      if (ps->current != NULL)
      {
         pos = safecat(buffer, bufsize, pos, ps->current->name);
         pos = safecat(buffer, bufsize, pos, sep);
      }
   }
   else if (pp == ps->pwrite)
   {
      /* Writing a file */
      pos = safecat(buffer, bufsize, pos, "write: ");
      pos = safecat(buffer, bufsize, pos, ps->wname);
   }
   else
   {
      /* Neither reading nor writing */
      pos = safecat(buffer, bufsize, pos, "pngvalid: ");
   }

   pos = safecat(buffer, bufsize, pos, ps->test);
   pos = safecat(buffer, bufsize, pos, " ");
   pos = safecat(buffer, bufsize, pos, msg);
   return pos;
}

/* Functions to use as PNG callbacks. */
static void
sucker_error(png_structp pp, png_const_charp message) /* PNG_NORETURN */
{
   png_sucker *ps = png_get_error_ptr(pp);
   char buffer[256];

   sucker_message(pp, buffer, sizeof buffer, message);

   if (ps->nerrors++ == 0)
      safecat(ps->error, sizeof ps->error, 0, buffer);

   if (ps->verbose)
      fprintf(stderr, "error: %s\n", buffer);

   /* The longjmp argument is because, by UTSL, libpng calls longjmp with 1, and
    * libpng is *not* expected to ever call longjmp, so this is a sanity
    * check.  The code below ensures that libpng gets a copy of our jmp_buf.
    */
   longjmp(ps->jmpbuf, SUCKER_ERROR);
}

static void
sucker_warning(png_structp pp, png_const_charp message)
{
   png_sucker *ps = png_get_error_ptr(pp);
   char buffer[256];

   sucker_message(pp, buffer, sizeof buffer, message);

   if (ps->nwarnings++ == 0 && ps->nerrors == 0)
      safecat(ps->error, sizeof ps->error, 0, buffer);

   if (ps->verbose)
      fprintf(stderr, "warning: %s\n", buffer);
}

static void
sucker_write(png_structp pp, png_bytep pb, png_size_t st)
{
   png_sucker *ps = png_get_io_ptr(pp);
   if (ps->pwrite != pp)
      png_error(pp, "sucker state damaged");
   while (st > 0)
   {
      size_t cb;

      if (ps->writepos >= SUCKER_BUFFER_SIZE)
         sucker_storenew(ps);

      cb = st;
      if (cb > SUCKER_BUFFER_SIZE - ps->writepos)
         cb = SUCKER_BUFFER_SIZE - ps->writepos;
      memcpy(ps->new.buffer + ps->writepos, pb, cb);
      pb += cb;
      st -= cb;
      ps->writepos += cb;
   }
}

static void
sucker_flush(png_structp pp)
{
   /*DOES NOTHING*/
}

static size_t
sucker_read_buffer_size(png_sucker *ps)
{
   /* Return the bytes available for read in the current buffer. */
   if (ps->next != &ps->current->data)
      return SUCKER_BUFFER_SIZE;

   return ps->current->datacount;
}

static int
sucker_read_buffer_next(png_sucker *ps)
{
   png_sucker_buffer *pbOld = ps->next;
   png_sucker_buffer *pbNew = &ps->current->data;
   if (pbOld != pbNew)
   {
      while (pbNew != NULL && pbNew->prev != pbOld)
         pbNew = pbNew->prev;
      if (pbNew != NULL)
      {
         ps->next = pbNew;
         ps->readpos = 0;
         return 1;
      }

      png_error(ps->pread, "buffer lost");
   }

   return 0; /* EOF or error */
}

static void
sucker_read(png_structp pp, png_bytep pb, png_size_t st)
{
   png_sucker *ps = png_get_io_ptr(pp);
   if (ps->pread != pp || ps->current == NULL || ps->next == NULL)
      png_error(pp, "sucker state damaged");
   while (st > 0)
   {
      size_t cbAvail = sucker_read_buffer_size(ps) - ps->readpos;

      if (cbAvail > 0)
      {
         if (cbAvail > st) cbAvail = st;
         memcpy(pb, ps->next->buffer + ps->readpos, cbAvail);
         st -= cbAvail;
         pb += cbAvail;
         ps->readpos += cbAvail;
      }
      else if (!sucker_read_buffer_next(ps))
         png_error(pp, "read beyond end of file");
   }
}

/* Setup functions. */
/* Cleanup when aborting a write or after storing the new file. */
static void
sucker_write_reset(png_sucker *ps)
{
   if (ps->pwrite != NULL)
   {
      png_destroy_write_struct(&ps->pwrite, &ps->piwrite);
      ps->pwrite = NULL;
      ps->piwrite = NULL;
   }
   
   sucker_freenew(ps);
}

/* The following is the main write function, it returns a png_struct and,
 * optionally, a png)info suitable for writiing a new PNG file.  Use
 * sucker_storefile above to record this file after it has been written.  The
 * returned libpng structures as destroyed by sucker_write_reset above.
 */
static png_structp
set_sucker_for_write(png_sucker *ps, png_infopp ppi, const char name[64])
{
   if (setjmp(ps->jmpbuf) != 0)
      return NULL;

   if (ps->pwrite != NULL)
      png_error(ps->pwrite, "sucker already in use");

   sucker_write_reset(ps);
   safecat(ps->wname, sizeof ps->wname, 0, name);

   ps->pwrite = png_create_write_struct(PNG_LIBPNG_VER_STRING, ps, sucker_error,
      sucker_warning);
   png_set_write_fn(ps->pwrite, ps, sucker_write, sucker_flush);

   if (ppi != NULL)
      *ppi = ps->piwrite = png_create_info_struct(ps->pwrite);

   return ps->pwrite;
}

/* Cleanup when finished reading (either due to error or in the success case. )
 */
static void
sucker_read_reset(png_sucker *ps)
{
   if (ps->pread != NULL)
   {
      png_destroy_read_struct(&ps->pread, &ps->piread, NULL);
      ps->pread = NULL;
      ps->piread = NULL;
   }

   ps->current = NULL;
   ps->next = NULL;
   ps->readpos = 0;
}

static void
sucker_read_set(png_sucker *ps, png_uint_32 id)
{
   png_sucker_file *pf = ps->saved;

   while (pf != NULL)
   {
      if (pf->id == id)
      {
         ps->current = pf;
         ps->next = NULL;
         sucker_read_buffer_next(ps);
         return;
      }

      pf = pf->next;
   }

   png_error(ps->pread, "unable to find file to read");
}

/* The main interface for reading a saved file - pass the id number of the file
 * to retrieve.  Ids must be unique or the earlier file will be hidden.  The API
 * returns a png_struct and, optionally, a png_info.  Both of these will be
 * destroyed by sucker_read_reset above.
 */
static png_structp
set_sucker_for_read(png_sucker *ps, png_infopp ppi, png_uint_32 id,
   const char *name)
{
   safecat(ps->test, sizeof ps->test, 0, name);

   if (setjmp(ps->jmpbuf) != 0)
      return NULL;

   if (ps->pread != NULL)
      png_error(ps->pread, "sucker already in use");

   sucker_read_reset(ps);

   ps->pread = png_create_read_struct(PNG_LIBPNG_VER_STRING, ps, sucker_error,
      sucker_warning);
   sucker_read_set(ps, id);
   png_set_read_fn(ps->pread, ps, sucker_read);

   if (ppi != NULL)
      *ppi = ps->piread = png_create_info_struct(ps->pread);

   return ps->pread;
}

/*********************** PNG FILE MODIFICATION ON READ ************************/
/* Files may be modified on read.  The following structure contains a complete
 * png_sucker together with extra members to handle modification and a special
 * read callback for libpng.  To use this the 'modifications' field must be set
 * to a list of png_modification structures that actually perform the
 * modification, otherwise a png_modifier is functionally equivalent to a
 * png_sucker.  There is a special read function, set_modifier_for_read, which
 * replaces set_sucker_for_read.
 */
typedef struct png_modifier
{
   png_sucker               this;            /* I am a png_sucker */
   struct png_modification *modifications;   /* Changes to make */
   enum modifier_state
   {
      modifier_start,                        /* Initial value */
      modifier_signature,                    /* Have a signature */
      modifier_IHDR                          /* Have an IHDR */
   }                        state;           /* My state */

   /* Information from IHDR: */
   png_byte                 bit_depth;       /* From IHDR */
   png_byte                 colour_type;     /* From IHDR */

   /* While handling PLTE, IDAT and IEND these chunks may be pended to allow
    * other chunks to be inserted.
    */
   png_uint_32              pending_len;
   png_uint_32              pending_chunk;

   /* Test values */
   double                  *gammas;
   unsigned                 ngammas;

   /* Lowest sbit to test (libpng fails for sbit < 8) */
   unsigned                 sbitlow;

   /* Error control - these are the limits on errors accepted by the gamma tests
    * below.
    */
   double                   maxout8;  /* Maximum output value error */
   double                   maxabs8;  /* Abosulte sample error 0..1 */
   double                   maxpc8;   /* Percentage sample error 0..100% */
   double                   maxout16; /* Maximum output value error */
   double                   maxabs16; /* Absolute sample error 0..1 */
   double                   maxpc16;  /* Percentage sample error 0..100% */

   /* Logged 8 and 16 bit errors ('output' values): */
   double                   error_gray_2;
   double                   error_gray_4;
   double                   error_gray_8;
   double                   error_gray_16;
   double                   error_color_8;
   double                   error_color_16;

   /* Flags: */
   /* When to use the use_input_precision option: */
   int                      use_input_precision :1;
   int                      use_input_precision_sbit :1;
   int                      use_input_precision_16to8 :1;
   int                      log :1;   /* Log max error */

   /* Buffer information, the buffer size limits the size of the chunks that can
    * be modified - they must fit (including header and CRC) into the buffer!
    */
   size_t                   flush;           /* Count of bytes to flush */
   size_t                   buffer_count;    /* Bytes in buffer */
   size_t                   buffer_position; /* Position in buffer */
   png_byte                 buffer[1024];
} png_modifier;

static double abserr(png_modifier *pm, png_byte bit_depth)
{
   return bit_depth == 16 ? pm->maxabs16 : pm->maxabs8;
}

static double pcerr(png_modifier *pm, png_byte bit_depth)
{
   return (bit_depth == 16 ? pm->maxpc16 : pm->maxpc8) * .01;
}

static double outerr(png_modifier *pm, png_byte bit_depth)
{
   /* There is a serious error in the 2 and 4 bit grayscale transform because
    * the gamma table value (8 bits) is simply shifted, not rouned, so the
    * error in 4 bit greyscale gamma is up to the value below.  This is a hack
    * to allow pngvalid to succeed:
    */
   if (bit_depth == 2)  return .73182-.5;
   if (bit_depth == 4)  return .90644-.5;
   if (bit_depth == 16) return pm->maxout16;
   return pm->maxout8;
}

/* This returns true if the test should be stopped now because it has already
 * failed and it is running silently.
 */
static int fail(png_modifier *pm)
{
   return !pm->log && !pm->this.verbose && (pm->this.nerrors > 0 ||
      pm->this.treat_warnings_as_errors && pm->this.nwarnings > 0);
}

static void
modifier_init(png_modifier *pm)
{
   memset(pm, 0, sizeof *pm);
   sucker_init(&pm->this);
   pm->modifications = NULL;
   pm->state = modifier_start;
   pm->sbitlow = 1;
   pm->maxout8 = pm->maxpc8 = pm->maxabs8 = 0;
   pm->maxout16 = pm->maxpc16 = pm->maxabs16 = 0;
   pm->error_gray_2 = pm->error_gray_4 = pm->error_gray_8 = 0;
   pm->error_gray_16 = pm->error_color_8 = pm->error_color_16 = 0;
   pm->use_input_precision = 0;
   pm->use_input_precision_sbit = 0;
   pm->use_input_precision_16to8 = 0;
   pm->log = 0;

   /* Rely on the memset for all the other fields - there are no pointers */
}

/* One modification strucutre must be provided for each chunk to be modified (in
 * fact more than one can be provided if multiple separate changes are desired
 * for a single chunk.)  Modifications include adding a new chunk when a
 * suitable chunk does not exist.
 *
 * The caller of modify_fn will reset the CRC of the chunk and record 'modified'
 * or 'added' as appropriate if the modify_fn returns 1 (true).  If the
 * modify_fn is NULL the chunk is simply removed.
 */
typedef struct png_modification
{
   struct png_modification *next;
   png_uint_32              chunk;
   /* If the following is NULL all matching chunks will be removed: */
   int                    (*modify_fn)(png_structp pp, struct png_modifier *pm,
                                       struct png_modification *me, int add);
   /* If the following is set to PLTE, IDAT or IEND and the chunk has not been
    * found and modified (and there is a modify_fn) the modify_fn will be called
    * to add the chunk before the relevant chunk.
    */
   png_uint_32              add;
   int                      modified :1;     /* Chunk was modified */
   int                      added    :1;     /* Chunk was added */
   int                      removed  :1;     /* Chunk was removed */
} png_modification;

static void modification_reset(png_modification *pmm)
{
   if (pmm != NULL)
   {
      pmm->modified = 0;
      pmm->added = 0;
      pmm->removed = 0;
      modification_reset(pmm->next);
   }
}

static void
modification_init(png_modification *pmm)
{
   memset(pmm, 0, sizeof *pmm);
   pmm->next = NULL;
   pmm->chunk = 0;
   pmm->modify_fn = NULL;
   pmm->add = 0;
   modification_reset(pmm);
}

static void
modifier_reset(png_modifier *pm)
{
   sucker_read_reset(&pm->this);
   pm->modifications = NULL;
   pm->state = modifier_start;
   pm->bit_depth = pm->colour_type = 0;
   pm->pending_len = pm->pending_chunk = 0;
   pm->flush = pm->buffer_count = pm->buffer_position = 0;
}

/* Convenience macros. */
#define CHUNK(a,b,c,d) (((a)<<24)+((b)<<16)+((c)<<8)+(d))
#define CHUNK_IHDR CHUNK(73,72,68,82)
#define CHUNK_PLTE CHUNK(80,76,84,69)
#define CHUNK_IDAT CHUNK(73,68,65,84)
#define CHUNK_IEND CHUNK(73,69,78,68)
#define CHUNK_cHRM CHUNK(99,72,82,77)
#define CHUNK_gAMA CHUNK(103,65,77,65)
#define CHUNK_sBIT CHUNK(115,66,73,84)
#define CHUNK_sRGB CHUNK(115,82,71,66)

/* The guts of modification are performed during a read. */
static void
modifier_crc(png_bytep buffer)
{
   /* Recalculate the chunk CRC - a complete chunk must be in
    * the buffer, at the start.
    */
   uInt datalen = png_get_uint_32(buffer);
   png_save_uint_32(buffer+datalen+8, crc32(0L, buffer+4, datalen+4));
}

static void
modifier_setbuffer(png_modifier *pm)
{
   modifier_crc(pm->buffer);
   pm->buffer_count = png_get_uint_32(pm->buffer)+12;
   pm->buffer_position = 0;
}

static void
modifier_read(png_structp pp, png_bytep pb, png_size_t st)
{
   png_modifier *pm = png_get_io_ptr(pp);

   while (st > 0)
   {
      size_t cb;
      png_uint_32 len, chunk;
      png_modification *mod;

      if (pm->buffer_position >= pm->buffer_count) switch (pm->state)
      {
      static png_byte sign[8] = { 137, 80, 78, 71, 13, 10, 26, 10 };
      case modifier_start:
         sucker_read(pp, pm->buffer, 8); /* size of signature. */
         pm->buffer_count = 8;
         pm->buffer_position = 0;

         if (memcmp(pm->buffer, sign, 8) != 0)
            png_error(pp, "invalid PNG file signature");
         pm->state = modifier_signature;
         break;

      case modifier_signature:
         sucker_read(pp, pm->buffer, 13+12); /* size of IHDR */
         pm->buffer_count = 13+12;
         pm->buffer_position = 0;

         if (png_get_uint_32(pm->buffer) != 13 ||
             png_get_uint_32(pm->buffer+4) != CHUNK_IHDR)
            png_error(pp, "invalid IHDR");

         /* Check the list of modifiers for modifications to the IHDR. */
         mod = pm->modifications;
         while (mod != NULL)
         {
            if (mod->chunk == CHUNK_IHDR && mod->modify_fn &&
                (*mod->modify_fn)(pp, pm, mod, 0))
               {
               mod->modified = 1;
               modifier_setbuffer(pm);
               }

            /* Ignore removal or add if IHDR! */
            mod = mod->next;
         }

         /* Cache information from the IHDR (the modified one.) */
         pm->bit_depth = pm->buffer[8+8];
         pm->colour_type = pm->buffer[8+8+1];

         pm->state = modifier_IHDR;
         pm->flush = 0;
         break;

      default:
         /* Read a new chunk and process it until we see PLTE, IDAT or
          * IEND.  'flush' indicates that there is still some data to
          * output from the preceding chunk.
          */
         if ((cb = pm->flush) > 0)
         {
            if (cb > st) cb = st;
            pm->flush -= cb;
            sucker_read(pp, pb, cb);
            pb += cb;
            st -= cb;
            if (st <= 0) return;
         }

         /* No more bytes to flush, read a header, or handle a pending
          * chunk.
          */
         if (pm->pending_chunk != 0)
         {
            png_save_uint_32(pm->buffer, pm->pending_len);
            png_save_uint_32(pm->buffer+4, pm->pending_chunk);
            pm->pending_len = 0;
            pm->pending_chunk = 0;
         }
         else
            sucker_read(pp, pm->buffer, 8);

         pm->buffer_count = 8;
         pm->buffer_position = 0;

         /* Check for something to modify or a terminator chunk. */
         len = png_get_uint_32(pm->buffer);
         chunk = png_get_uint_32(pm->buffer+4);

         /* Terminators first, they may have to be delayed for added
          * chunks
          */
         if (chunk == CHUNK_PLTE || chunk == CHUNK_IDAT || chunk == CHUNK_IEND)
         {
            mod = pm->modifications;

            while (mod != NULL)
            {
               if ((mod->add == chunk ||
                   mod->add == CHUNK_PLTE && chunk == CHUNK_IDAT) &&
                   mod->modify_fn != NULL && !mod->modified && !mod->added)
               {
                  /* Regardless of what the modify function does do not run this
                   * again.
                   */
                  mod->added = 1;

                  if ((*mod->modify_fn)(pp, pm, mod, 1/*add*/))
                  {
                     /* Reset the CRC on a new chunk */
                     if (pm->buffer_count > 0)
                        modifier_setbuffer(pm);
                     else
                        {
                        pm->buffer_position = 0;
                        mod->removed = 1;
                        }

                     /* The buffer has been filled with something (we assume) so
                      * output this.  Pend the current chunk.
                      */
                     pm->pending_len = len;
                     pm->pending_chunk = chunk;
                     break; /* out of while */
                  }
               }

               mod = mod->next;
            }

            /* Don't do any further processing if the buffer was modified -
             * otherwise the code will end up modifying a chunk that was just
             * added.
             */
            if (mod != NULL)
               break; /* out of switch */
         }

         /* If we get to here then this chunk may need to be modified.  To do
          * this is must be less than 1024 bytes in total size, otherwise
          * it just gets flushed.
          */
         if (len+12 <= sizeof pm->buffer)
         {
            sucker_read(pp, pm->buffer+pm->buffer_count,
               len+12-pm->buffer_count);
            pm->buffer_count = len+12;

            /* Check for a modification, else leave it be. */
            mod = pm->modifications;
            while (mod != NULL)
            {
               if (mod->chunk == chunk)
               {
                  if (mod->modify_fn == NULL)
                  {
                     /* Remove this chunk */
                     pm->buffer_count = pm->buffer_position = 0;
                     mod->removed = 1;
                     break; /* Terminate the while loop */
                  }
                  else if ((*mod->modify_fn)(pp, pm, mod, 0))
                  {
                     mod->modified = 1;
                     /* The chunk may have been removed: */
                     if (pm->buffer_count == 0)
                     {
                        pm->buffer_position = 0;
                        break;
                     }
                     modifier_setbuffer(pm);
                  }
               }

               mod = mod->next;
            }
         }
         else
            pm->flush = len+12 - pm->buffer_count; /* data + crc */

         /* Take the data from the buffer (if there is any). */
         break;
      }

      /* Here to read from the modifier buffer (not directly from
       * the sucker, as in the flush case above.)
       */
      cb = pm->buffer_count - pm->buffer_position;
      if (cb > st) cb = st;
      memcpy(pb, pm->buffer + pm->buffer_position, cb);
      st -= cb;
      pb += cb;
      pm->buffer_position += cb;
   }
}

/* Set up a modifier. */
static png_structp
set_modifier_for_read(png_modifier *pm, png_infopp ppi, png_uint_32 id,
   const char *name)
{
   png_structp pp = set_sucker_for_read(&pm->this, ppi, id, name);

   if (pp != NULL)
   {
      if (setjmp(pm->this.jmpbuf) == 0)
      {
         png_set_read_fn(pp, pm, modifier_read);

         pm->state = modifier_start;
         pm->bit_depth = 0;
         pm->colour_type = 255;

         pm->pending_len = 0;
         pm->pending_chunk = 0;
         pm->flush = 0;
         pm->buffer_count = 0;
         pm->buffer_position = 0;
      }
      else
      {
         sucker_read_reset(&pm->this);
         pp = NULL;
      }
   }

   return pp;
}

/***************************** STANDARD PNG FILES *****************************/
/* Standard files - write and save standard files. */
/* The standard files are constructed with rows which fit into a 1024 byte row
 * buffer.  This makes allocation easier below.  Further regardless of the file
 * format every file has 128 pixels (giving 1024 bytes for 64bpp formats).
 *
 * Files are stored with no gAMA or sBIT chunks, with a PLTE only when needed
 * and with an ID derived from the colour type and bit depth as follows:
 */
#define FILEID(col, depth) ((png_uint_32)((col) + ((depth)<<3)))
#define COL_FROM_ID(id) ((id)& 0x7)
#define DEPTH_FROM_ID(id) (((id) >> 3) & 0x1f)

#define STD_WIDTH  128
#define STD_ROWMAX (STD_WIDTH*8)

static unsigned
bit_size(png_structp pp, png_byte colour_type, png_byte bit_depth)
{
   switch (colour_type)
   {
   case 0:  return bit_depth;
   case 2:  return 3*bit_depth;
   case 3:  return bit_depth;
   case 4:  return 2*bit_depth;
   case 6:  return 4*bit_depth;
   default: png_error(pp, "invalid color type");
   }
}

static size_t
standard_rowsize(png_structp pp, png_byte colour_type, png_byte bit_depth)
{
   return (STD_WIDTH * bit_size(pp, colour_type, bit_depth)) / 8;
}

static png_uint_32
standard_width(png_structp pp, png_byte colour_type, png_byte bit_depth)
{
   return STD_WIDTH;
}

static png_uint_32
standard_height(png_structp pp, png_byte colour_type, png_byte bit_depth)
{
   switch (bit_size(pp, colour_type, bit_depth))
   {
   case 1:
   case 2:
   case 4:
      return 1;   /* Total of 128 pixels */
   case 8:
      return 2;   /* Total of 256 pixels/bytes */
   case 16:
      return 512; /* Total of 65536 pixels */
   case 24:
   case 32:
      return 512; /* 65536 pixels */
   case 48:
   case 64:
      return 2048;/* 4 x 65536 pixels. */
   }
}

static void
standard_row(png_structp pp, png_byte buffer[STD_ROWMAX], png_byte colour_type,
   png_byte bit_depth, png_uint_32 y)
{
   png_uint_32 v = y << 7;
   png_uint_32 i = 0;

   switch (bit_size(pp, colour_type, bit_depth))
   {
   case 1:
      while (i<128/8) buffer[i] = v & 0xff, v += 17, ++i;
      return;
   case 2:
      while (i<128/4) buffer[i] = v & 0xff, v += 33, ++i;
      return;
   case 4:
      while (i<128/2) buffer[i] = v & 0xff, v += 65, ++i;
      return;
   case 8:
      /* 256 bytes total, 128 bytes in each row set as follows: */
      while (i<128) buffer[i] = v & 0xff, ++v, ++i;
      return;
   case 16:
      /* Generate all 65536 pixel values in order, this includes the 8 bit GA
       * case as we as the 16 bit G case.
       */
      while (i<128)
         buffer[2*i] = (v>>8) & 0xff, buffer[2*i+1] = v & 0xff, ++v, ++i;
      return;
   case 24:
      /* 65535 pixels, but rotate the values. */
      while (i<128)
      {
         /* Three bytes per pixel, r, g, b, make b by r^g */
         buffer[3*i+0] = (v >> 8) & 0xff;
         buffer[3*i+1] = v & 0xff;
         buffer[3*i+2] = ((v >> 8) ^ v) & 0xff;
         ++v;
         ++i;
      }
      return;
   case 32:
      /* 65535 pixels, r, g, b, a; just replicate */
      while (i<128)
      {
         buffer[4*i+0] = (v >> 8) & 0xff;
         buffer[4*i+1] = v & 0xff;
         buffer[4*i+2] = (v >> 8) & 0xff;
         buffer[4*i+3] = v & 0xff;
         ++v;
         ++i;
      }
      return;
   case 48:
      /* y is maximum 2047, giving 4x65536 pixels, make 'r' increase by 1 at
       * each pixel, g increase by 257 (0x101) and 'b' by 0x1111:
       */
      while (i<128)
      {
         png_uint_32 t = v++;
         buffer[6*i+0] = (t >> 8) & 0xff;
         buffer[6*i+1] = t & 0xff;
         t *= 257;
         buffer[6*i+2] = (t >> 8) & 0xff;
         buffer[6*i+3] = t & 0xff;
         t *= 17;
         buffer[6*i+4] = (t >> 8) & 0xff;
         buffer[6*i+5] = t & 0xff;
         ++i;
      }
      return;
   case 64:
      /* As above in the 32 bit case. */
      while (i<128)
      {
         png_uint_32 t = v++;
         buffer[8*i+0] = (t >> 8) & 0xff;
         buffer[8*i+1] = t & 0xff;
         buffer[8*i+4] = (t >> 8) & 0xff;
         buffer[8*i+5] = t & 0xff;
         t *= 257;
         buffer[8*i+2] = (t >> 8) & 0xff;
         buffer[8*i+3] = t & 0xff;
         buffer[8*i+6] = (t >> 8) & 0xff;
         buffer[8*i+7] = t & 0xff;
         ++i;
      }
      return;
   }

   png_error(pp, "internal error");
}

static void
make_standard(png_sucker* ps, png_byte colour_type, int bdlo, int bdhi)
{
   for (; bdlo <= bdhi; ++bdlo)
   {
      png_byte bit_depth = 1U << bdlo;
      png_uint_32 h, y;
      png_structp pp;
      png_infop pi;

      {
         size_t pos;
         char name[64];  /* Same size as the buffer in a file. */

         /* Build a name */
         pos = safecat(name, sizeof name, 0, bit_depths[bdlo]);
         pos = safecat(name, sizeof name, pos, "bit ");
         pos = safecat(name, sizeof name, pos, colour_types[colour_type]);

         /* Get a png_struct for writing the image. */
         pp = set_sucker_for_write(ps, &pi, name);
      }
      if (pp == NULL) return;

      /* Do the honourable write stuff, protected by a local setjmp */
      if (setjmp(ps->jmpbuf) != 0)
      {
         sucker_write_reset(ps);
         continue;
      }

      h = standard_height(pp, colour_type, bit_depth),
      png_set_IHDR(pp, pi, standard_width(pp, colour_type, bit_depth), h,
         bit_depth, colour_type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE,
         PNG_FILTER_TYPE_BASE);

      if (colour_type == 3) /* palette */
      {
         int i;
         png_color pal[256];
         for (i=0; i<256; ++i) pal[i].red = pal[i].green = pal[i].blue = i;
         png_set_PLTE(pp, pi, pal, 256);
      }

      png_write_info(pp, pi);

      if (png_get_rowbytes(pp, pi) !=
          standard_rowsize(pp, colour_type, bit_depth))
         png_error(pp, "row size incorrect");

      else for (y=0; y<h; ++y)
      {
         png_byte buffer[STD_ROWMAX];
         standard_row(pp, buffer, colour_type, bit_depth, y);
         png_write_row(pp, buffer);
      }

      png_write_end(pp, pi);

      /* And store this under the appropriate id, then clean up. */
      sucker_storefile(ps, FILEID(colour_type, bit_depth));

      sucker_write_reset(ps);
   }
}

static void
make_standard_images(png_sucker *ps)
{
   /* Arguments are colour_type, low bit depth, high bit depth */
   make_standard(ps, 0, 0, 4);
   make_standard(ps, 2, 3, 4);
   make_standard(ps, 3, 0, 3);
   make_standard(ps, 4, 3, 4);
   make_standard(ps, 6, 3, 4);
}

/* Tests - individual test cases */
static void
test_standard(png_sucker* ps, png_byte colour_type, int bdlo, int bdhi)
{
   for (; bdlo <= bdhi; ++bdlo)
   {
      png_byte bit_depth = 1U << bdlo;
      png_uint_32 h, y;
      size_t cb;
      png_structp pp;
      png_infop pi;

      /* Get a png_struct for writing the image. */
      pp = set_sucker_for_read(ps, &pi, FILEID(colour_type, bit_depth),
         "standard");
      if (pp == NULL) return;

      /* Do the honourable write stuff, protected by a local setjmp */
      if (setjmp(ps->jmpbuf) != 0)
      {
         sucker_read_reset(ps);
         continue;
      }

      h = standard_height(pp, colour_type, bit_depth);

      /* Check the header values: */
      png_read_info(pp, pi);

      if (png_get_image_width(pp, pi) !=
          standard_width(pp, colour_type, bit_depth))
         png_error(pp, "validate: image width changed");
      if (png_get_image_height(pp, pi) != h)
         png_error(pp, "validate: image height changed");
      if (png_get_bit_depth(pp, pi) != bit_depth)
         png_error(pp, "validate: bit depth changed");
      if (png_get_color_type(pp, pi) != colour_type)
         png_error(pp, "validate: color type changed");
      if (png_get_filter_type(pp, pi) != PNG_FILTER_TYPE_BASE)
         png_error(pp, "validate: filter type changed");
      if (png_get_interlace_type(pp, pi) != PNG_INTERLACE_NONE)
         png_error(pp, "validate: interlacing changed");
      if (png_get_compression_type(pp, pi) != PNG_COMPRESSION_TYPE_BASE)
         png_error(pp, "validate: compression type changed");
      if (png_set_interlace_handling(pp) != 1)
         png_error(pp, "validate: interlacing unexpected");

      if (colour_type == 3) /* palette */
      {
         png_colorp pal;
         int num;
         if (png_get_PLTE(pp, pi, &pal, &num) & PNG_INFO_PLTE)
         {
            int i;
            if (num != 256)
               png_error(pp, "validate: color type 3 PLTE chunk size changed");
            for (i=0; i<num; ++i)
               if (pal[i].red != i || pal[i].green != i || pal[i].blue != i)
                  png_error(pp, "validate: color type 3 PLTE chunk changed");
         }
         else
            png_error(pp, "validate: missing PLTE with color type 3");
      }

      cb = standard_rowsize(pp, colour_type, bit_depth);
      png_start_read_image(pp);

      if (png_get_rowbytes(pp, pi) != cb)
         png_error(pp, "validate: row size changed");

      else for (y=0; y<h; ++y)
      {
         png_byte std[STD_ROWMAX];
         png_byte read[STD_ROWMAX];
         png_byte display[STD_ROWMAX];

         standard_row(pp, std, colour_type, bit_depth, y);
         png_read_row(pp, read, display);

         if (memcmp(std, read, cb) != 0)
         {
            char msg[64];
            sprintf(msg, "validate: PNG image row %d (of %d) changed", y,
               h);
            png_error(pp, msg);
         }
         if (memcmp(std, display, cb) != 0)
         {
            char msg[64];
            sprintf(msg, "validate: transformed row %d (of %d) changed", y, h);
            png_error(pp, msg);
         }
      }

      png_read_end(pp, pi);

      sucker_read_reset(ps);
   }
}

static void
perform_standard_test(png_modifier *pm)
{
   test_standard(&pm->this, 0, 0, 4);
   if (fail(pm)) return;
   test_standard(&pm->this, 2, 3, 4);
   if (fail(pm)) return;
   test_standard(&pm->this, 3, 0, 3);
   if (fail(pm)) return;
   test_standard(&pm->this, 4, 3, 4);
   if (fail(pm)) return;
   test_standard(&pm->this, 6, 3, 4);
}


/********************************* GAMMA TESTS ********************************/
/* Gamma test images. */
static void
make_gamma_images(png_sucker *ps)
{
   /* Do nothing - the standard greyscale images are used. */
}

typedef struct gamma_modification
{
   png_modification this;
   png_fixed_point  gamma;
}
gamma_modification;

static int
gamma_modify(png_structp pp, png_modifier *pm, png_modification *me, int add)
{
   /* This simply dumps the given gamma value into the buffer. */
   png_save_uint_32(pm->buffer, 4);
   png_save_uint_32(pm->buffer+4, CHUNK_gAMA);
   png_save_uint_32(pm->buffer+8, ((gamma_modification*)me)->gamma);
   return 1;
}

static void
gamma_modification_init(gamma_modification *me, png_modifier *pm, double gamma)
{
   modification_init(&me->this);
   me->this.chunk = CHUNK_gAMA;
   me->this.modify_fn = gamma_modify;
   me->this.add = CHUNK_PLTE;
   me->gamma = floor(gamma * 100000 + .5);
   me->this.next = pm->modifications;
   pm->modifications = &me->this;
}

typedef struct srgb_modification
{
   png_modification this;
   png_byte         intent;
}
srgb_modification;

static int
srgb_modify(png_structp pp, png_modifier *pm, png_modification *me, int add)
{
   /* As above, ignore add and just make a new chunk */
   png_save_uint_32(pm->buffer, 1);
   png_save_uint_32(pm->buffer+4, CHUNK_sRGB);
   pm->buffer[8] = ((srgb_modification*)me)->intent;
   return 1;
}

static void
srgb_modification_init(srgb_modification *me, png_modifier *pm, png_byte intent)
{
   modification_init(&me->this);
   me->this.chunk = CHUNK_sBIT;
   if (intent <= 3) /* if valid, else *delete* sRGB chunks */
   {
      me->this.modify_fn = srgb_modify;
      me->this.add = CHUNK_PLTE;
      me->intent = intent;
   }
   else
   {
      me->this.modify_fn = 0;
      me->this.add = 0;
      me->intent = 0;
   }
   me->this.next = pm->modifications;
   pm->modifications = &me->this;
}

typedef struct sbit_modification
{
   png_modification this;
   png_byte         sbit;
}
sbit_modification;

static int
sbit_modify(png_structp pp, png_modifier *pm, png_modification *me, int add)
{
   png_byte sbit = ((sbit_modification*)me)->sbit;
   if (pm->bit_depth > sbit)
   {
      int cb = 0;
      switch (pm->colour_type)
      {
      case 0: cb = 1; break;
      case 2:
      case 3: cb = 3; break;
      case 4: cb = 2; break;
      case 6: cb = 4; break;
      default:
         png_error(pp, "unexpected colour type in sBIT modification");
      }

      png_save_uint_32(pm->buffer, cb);
      png_save_uint_32(pm->buffer+4, CHUNK_sBIT);
      while (cb > 0)
         (pm->buffer+8)[--cb] = sbit;

      return 1;
   }
   else if (!add)
   {
      /* Remove the sBIT chunk */
      pm->buffer_count = pm->buffer_position = 0;
      return 1;
   }
   else
      return 0; /* do nothing */
}

static void
sbit_modification_init(sbit_modification *me, png_modifier *pm, png_byte sbit)
{
   modification_init(&me->this);
   me->this.chunk = CHUNK_sBIT;
   me->this.modify_fn = sbit_modify;
   me->this.add = CHUNK_PLTE;
   me->sbit = sbit;
   me->this.next = pm->modifications;
   pm->modifications = &me->this;
}

/* maxabs: maximum absolute error as a fraction
 * maxout: maximum output error in the output units
 * maxpc:  maximum percentage error (as a percentage)
 */
static void
gamma_test(png_modifier *pm, const png_byte colour_type,
   const png_byte bit_depth, const double file_gamma, const double screen_gamma,
   const png_byte sbit, const int threshold_test, const char *name,
   const int speed, const int use_input_precision, const int strip16)
{
   png_structp pp;
   png_infop pi;
   double maxerrout = 0, maxerrpc = 0, maxerrabs = 0;

   gamma_modification gamma_mod;
   srgb_modification srgb_mod;
   sbit_modification sbit_mod;

   /* Make an appropriate modifier to set the PNG file gamma to the
    * given gamma value and the sBIT chunk to the given precision.
    */
   pm->modifications = NULL;
   gamma_modification_init(&gamma_mod, pm, file_gamma);
   srgb_modification_init(&srgb_mod, pm, 127/*delete*/);
   sbit_modification_init(&sbit_mod, pm, sbit);

   modification_reset(pm->modifications);

   /* Get a png_struct for writing the image. */
   pp = set_modifier_for_read(pm, &pi, FILEID(colour_type, bit_depth), name);
   if (pp == NULL) return;

   /* Do the honourable write stuff, protected by a local setjmp */
   if (setjmp(pm->this.jmpbuf) != 0)
   {
      modifier_reset(pm);
      return;
   }

   /* Set up gamma processing. */
   png_set_gamma(pp, screen_gamma, file_gamma);

   /* Check the header values: */
   png_read_info(pp, pi);

   /* If requested strip 16 to 8 bits - this is handled automagically below
    * because the output bit depth is read from the library.  Note that there
    * are interactions with sBIT but, internally, libpng makes sbit at most
    * PNG_MAX_GAMMA_8 when doing the following.
    */
   if (strip16)
      png_set_strip_16(pp);

   if (png_set_interlace_handling(pp) != 1)
      png_error(pp, "gamma: interlaced images not supported");

   png_read_update_info(pp, pi);

   {
      const png_byte out_ct = png_get_color_type(pp, pi);
      const png_byte out_bd = png_get_bit_depth(pp, pi);
      const unsigned outmax = (1U<<out_bd)-1;
      const png_uint_32 w = png_get_image_width(pp, pi);
      const png_uint_32 h = png_get_image_height(pp, pi);
      const size_t cb = png_get_rowbytes(pp, pi); /* For the memcmp below. */
      const double maxabs = abserr(pm, out_bd);
      const double maxout = outerr(pm, out_bd);
      const double maxpc = pcerr(pm, out_bd);
      png_uint_32 y;

      /* There are three sources of error, firstly the quantization in the file
       * encoding, determined by sbit and/or the file depth, secondly the output
       * (screen) gamma and thirdly the output file encoding.  Since this API
       * receives the screen and file gamma in double precision it is possible
       * to calculate an exact answer given an input pixel value.  Therefore we
       * assume that the *input* value is exact - sample/maxsample - calculate
       * the corresponding gamma corrected output to the limits of double
       * precision arithmetic and compare with what libpng returns.
       *
       * Since the library must quantise the output to 8 or 16 bits there is a
       * fundamental limit on the accuracy of the output of +/-.5 - this
       * quantisation limit is included in addition to the other limits
       * specified by the paramaters to the API.  (Effectively, add .5
       * everywhere.)
       *
       * The behavior of the 'sbit' paramter is defined by section 12.5 (sample
       * depth scaling) of the PNG spec.  That section forces the decoder to
       * assume that the PNG values have been scaled if sBIT is presence:
       *
       *     png-sample = floor( input-sample * (max-out/max-in) + .5 );
       *
       * This means that only a subset of the possible PNG values should appear
       * in the input, however the spec allows the encoder to use a variety of
       * approximations to the above and doesn't require any restriction of the
       * values produced.
       *
       * Nevertheless the spec requires that the upper 'sBIT' bits of the value
       * stored in a PNG file be the original sample bits.  Consequently the
       * code below simply scales the top sbit bits by (1<<sbit)-1 to obtain an
       * original sample value.
       *
       * Because there is limited precision in the input it is arguable that an
       * acceptable result is any valid result from input-.5 to input+.5.  The
       * basic tests below do not do this, however if 'use_input_precision' is
       * set a subsequent test is performed below.
       */
      const int processing = (fabs(screen_gamma*file_gamma-1) >=
         PNG_GAMMA_THRESHOLD && !threshold_test && !speed && colour_type != 3)
         || bit_depth != out_bd;
      const int samples_per_pixel = (out_ct & 2) ? 3 : 1;
      const double gamma = 1/(file_gamma*screen_gamma); /* Overall correction */

      for (y=0; y<h; ++y) /* just one pass - no interlacing */
      {
         unsigned s, x;
         png_byte std[STD_ROWMAX];
         png_byte display[STD_ROWMAX];

         standard_row(pp, std, colour_type, bit_depth, y);
         png_read_row(pp, NULL, display);

         if (processing) for (x=0; x<w; ++x) for (s=0; s<samples_per_pixel; ++s)
         {
            /* Input sample values: */
            const unsigned id = sample(std, colour_type, bit_depth, x, s);
            const unsigned od = sample(display, out_ct, out_bd, x, s);
            const unsigned isbit = id >> (bit_depth-sbit);
            double i, sample, encoded_sample, output, encoded_error, error;
            double es_lo, es_hi;

            /* First check on the 'perfect' result obtained from the digitized
             * input value, id, and compare this against the actual digitized
             * result, 'od'.  'i' is the input result in the range 0..1:
             *
             * NOTE: sbit should be taken into account here but isn't, as
             * described above.
             */
            i = isbit; i /= (1U<<sbit)-1;

            /* Then get the gamma corrected version of 'i' and compare to 'od',
             * any error less than .5 is insignificant - just quantization of
             * the output value to the nearest digital value (neverthelss the
             * error is still recorded - it's interesting ;-)
             */
            encoded_sample = pow(i, gamma) * outmax;
            encoded_error = fabs(od-encoded_sample);

            if (encoded_error > maxerrout)
               maxerrout = encoded_error;

            if (encoded_error < .5+maxout)
               continue;

            /* There may be an error, calculate the actual sample values -
             * unencoded light intensity values.  Note that in practice these
             * are not unencoded because they include a 'viewing correction' to
             * decrease or (normally) increase the perceptual contrast of the
             * image.  There's nothing we can do about this - we don't know what
             * it is - so assume the unencoded value is perceptually linear.
             */
            sample = pow(i, 1/file_gamma); /* In range 0..1 */
            output = od;
            output /= outmax;
            output = pow(output, screen_gamma);

            /* Now we have the numbers for real errors, both absolute values as
             * as a percentage of the correct value (output):
             */
            error = fabs(sample-output);
            if (error > maxerrabs)
               maxerrabs = error;
            /* The following is an attempt to ignore the tendency of
             * quantization to dominate the percentage errors for low output
             * sample values:
             */
            if (sample*maxpc > .5+maxabs)
            {
               double pcerr = error/sample;
               if (pcerr > maxerrpc) maxerrpc = pcerr;
            }

            /* Now calculate the digitization limits for 'encoded_sample' using
             * the 'max' values.  Note that maxout is in the encoded space but
             * maxpc and maxabs are in linear light space.
             *
             * First find the maximum error in linear light space, range 0..1:
             */
            {
               double tmp = sample * maxpc;
               if (tmp < maxabs) tmp = maxabs;

               /* Low bound - the minimum of the three: */
               es_lo = encoded_sample - maxout;
               if (es_lo > 0 && sample-tmp > 0)
               {
                  double l = outmax * pow(sample-tmp, 1/screen_gamma);
                  if (l < es_lo) es_lo = l;
               }
               else
                  es_lo = 0;

               es_hi = encoded_sample + maxout;
               if (es_hi < outmax && sample+tmp < 1)
               {
                  double h = outmax * pow(sample+tmp, 1/screen_gamma);
                  if (h > es_hi) es_hi = h;
               }
               else
                  es_hi = outmax;
            }

            /* The primary test is that the final encoded value returned by the
             * library should be between the two limits (inclusive) that were
             * calculated above.  At this point quantization of the output must
             * be taken into account.
             */
            if (od+.5 < es_lo || od-.5 > es_hi)
            {
               /* Thee has been an error in processing. */
               double is_lo, is_hi;

               if (use_input_precision)
               {
                  /* Ok, something is wrong - this actually happens in current
                   * libpng sbit processing.  Assume that the input value (id,
                   * adjusted for sbit) can be anywhere between value-.5 and
                   * value+.5 - quite a large range if sbit is low.
                   */
                  double tmp = (isbit - .5)/((1U<<sbit)-1);
                  if (tmp > 0)
                  {
                     is_lo = outmax * pow(tmp, gamma) - maxout;
                     if (is_lo < 0) is_lo = 0;
                  }
                  else
                     is_lo = 0;

                  tmp = (isbit + .5)/((1U<<sbit)-1);
                  if (tmp < 1)
                  {
                     is_hi = outmax * pow(tmp, gamma) + maxout;
                     if (is_hi > outmax) is_hi = outmax;
                  }
                  else
                     is_hi = outmax;

                  if (!(od+.5 < is_lo || od-.5 > is_hi))
                     continue;
               }

               {
                  char msg[256];
                  sprintf(msg,
                     "error: %.3f; %u{%u;%u} -> %u not %.2f (%.1f-%.1f)",
                     od-encoded_sample, id, sbit, isbit, od, encoded_sample,
                     use_input_precision ? is_lo : es_lo,
                     use_input_precision ? is_hi : es_hi);
                  png_warning(pp, msg);
               }
            }
         }
         else if (!speed && memcmp(std, display, cb) != 0)
         {
            char msg[64];
            /* No transform is expected on the threshold tests. */
            sprintf(msg, "gamma: below threshold row %d (of %d) changed", y, h);
            png_error(pp, msg);
         }
      }
   }

   png_read_end(pp, pi);
   modifier_reset(pm);

   if (pm->log && !threshold_test && !speed)
      fprintf(stderr, "%d bit %s %s: max error %f (%.2g, %2g%%)\n", bit_depth,
         colour_types[colour_type], name, maxerrout, maxerrabs, 100*maxerrpc);

   /* Log the summary values too. */
   if (colour_type == 0 || colour_type == 4) switch (bit_depth)
   {
   case 2:
      if (maxerrout > pm->error_gray_2) pm->error_gray_2 = maxerrout; break;
   case 4:
      if (maxerrout > pm->error_gray_4) pm->error_gray_4 = maxerrout; break;
   case 8:
      if (maxerrout > pm->error_gray_8) pm->error_gray_8 = maxerrout; break;
   case 16:
      if (maxerrout > pm->error_gray_16) pm->error_gray_16 = maxerrout; break;
   }
   else if (colour_type == 2 || colour_type == 6) switch (bit_depth)
   {
   case 8:
      if (maxerrout > pm->error_color_8) pm->error_color_8 = maxerrout; break;
   case 16:
      if (maxerrout > pm->error_color_16) pm->error_color_16 = maxerrout; break;
   }
}

static void gamma_threshold_test(png_modifier *pm, png_byte colour_type,
   png_byte bit_depth, double file_gamma, double screen_gamma)
{
   size_t pos = 0;
   char name[64];
   pos = safecat(name, sizeof name, pos, "threshold ");
   pos = safecatd(name, sizeof name, pos, file_gamma, 3);
   pos = safecat(name, sizeof name, pos, "/");
   pos = safecatd(name, sizeof name, pos, screen_gamma, 3);

   (void)gamma_test(pm, colour_type, bit_depth, file_gamma, screen_gamma,
      bit_depth, 1, name, 0/*speed*/, 0/*no input precision*/, 0/*no strip16*/);
}

static void
perform_gamma_threshold_tests(png_modifier *pm)
{
   png_byte colour_type = 0;
   png_byte bit_depth = 0;

   while (next_format(&colour_type, &bit_depth))
   {
      double gamma = 1.0;
      while (gamma >= .4)
      {
         gamma_threshold_test(pm, colour_type, bit_depth, gamma, 1/gamma);
         gamma *= .95;
      }

      /* And a special test for sRGB */
      gamma_threshold_test(pm, colour_type, bit_depth, .45455, 2.2);
      if (fail(pm)) return;
   }
}

static void gamma_transform_test(png_modifier *pm, const png_byte colour_type,
   const png_byte bit_depth, const double file_gamma, const double screen_gamma,
   const png_byte sbit, const int speed, const int use_input_precision,
   const int strip16)
{
   size_t pos = 0;
   char name[64];
   if (sbit != bit_depth)
   {
      pos = safecat(name, sizeof name, pos, "sbit(");
      pos = safecatn(name, sizeof name, pos, sbit);
      pos = safecat(name, sizeof name, pos, ") ");
   }
   else
      pos = safecat(name, sizeof name, pos, "gamma ");
   if (strip16)
      pos = safecat(name, sizeof name, pos, "16to8 ");
   pos = safecatd(name, sizeof name, pos, file_gamma, 3);
   pos = safecat(name, sizeof name, pos, "->");
   pos = safecatd(name, sizeof name, pos, screen_gamma, 3);

   gamma_test(pm, colour_type, bit_depth, file_gamma, screen_gamma, sbit, 0,
      name, speed, use_input_precision, strip16);
}

static void perform_gamma_transform_tests(png_modifier *pm, int speed)
{
   png_byte colour_type = 0;
   png_byte bit_depth = 0;

   /* Ignore palette images - the gamma correction happens on the palette entry,
    * haven't got the tests for this yet.
    */
   while (next_format(&colour_type, &bit_depth)) if (colour_type != 3)
   {
      int i, j;

      for (i=0; i<pm->ngammas; ++i) for (j=0; j<pm->ngammas; ++j) if (i != j)
      {
         gamma_transform_test(pm, colour_type, bit_depth, 1/pm->gammas[i],
            pm->gammas[j], bit_depth, speed, pm->use_input_precision,
            0/*do not strip16*/);
         if (fail(pm)) return;
      }
   }
}

static void perform_gamma_sbit_tests(png_modifier *pm, int speed)
{
   png_byte sbit;

   /* The only interesting cases are colour and grayscale, alpha is ignored here
    * for overall speed.  Only bit depths 8 and 16 are tested.
    */
   for (sbit=pm->sbitlow; sbit<16; ++sbit)
   {
      int i, j;
      for (i=0; i<pm->ngammas; ++i) for (j=0; j<pm->ngammas; ++j)
         if (i != j)
      {
         if (sbit < 8)
         {
            gamma_transform_test(pm, 0, 8, 1/pm->gammas[i], pm->gammas[j], sbit,
               speed, pm->use_input_precision_sbit, 0/*strip16*/);
            if (fail(pm)) return;
            gamma_transform_test(pm, 2, 8, 1/pm->gammas[i], pm->gammas[j], sbit,
               speed, pm->use_input_precision_sbit, 0/*strip16*/);
            if (fail(pm)) return;
         }
         gamma_transform_test(pm, 0, 16, 1/pm->gammas[i], pm->gammas[j], sbit,
            speed, pm->use_input_precision_sbit, 0/*strip16*/);
         if (fail(pm)) return;
         gamma_transform_test(pm, 2, 16, 1/pm->gammas[i], pm->gammas[j], sbit,
            speed, pm->use_input_precision_sbit, 0/*strip16*/);
         if (fail(pm)) return;
      }
   }
}

static void perform_gamma_strip16_tests(png_modifier *pm, int speed)
{
#  ifndef PNG_MAX_GAMMA_8
#     define PNG_MAX_GAMMA_8 11
#  endif
   /* Include the alpha cases here, not that sbit matches the internal value
    * used by the library - otherwise we will get spurious errors from the
    * internal sbit style approximation.
    *
    * The threshold test is here because otherwise the 16 to 8 convertion will
    * proceed *without* gamma correction, and the tests above will fail (but not
    * by much) - this could be fixed, it only appears with the -g option.
    */
   int i, j;
   for (i=0; i<pm->ngammas; ++i) for (j=0; j<pm->ngammas; ++j)
      if (i != j && fabs(pm->gammas[j]/pm->gammas[i]-1) >= PNG_GAMMA_THRESHOLD)
   {
      gamma_transform_test(pm, 0, 16, 1/pm->gammas[i], pm->gammas[j],
         PNG_MAX_GAMMA_8, speed, pm->use_input_precision_16to8, 1/*strip16*/);
      if (fail(pm)) return;
      gamma_transform_test(pm, 2, 16, 1/pm->gammas[i], pm->gammas[j],
         PNG_MAX_GAMMA_8, speed, pm->use_input_precision_16to8, 1/*strip16*/);
      if (fail(pm)) return;
      gamma_transform_test(pm, 4, 16, 1/pm->gammas[i], pm->gammas[j],
         PNG_MAX_GAMMA_8, speed, pm->use_input_precision_16to8, 1/*strip16*/);
      if (fail(pm)) return;
      gamma_transform_test(pm, 6, 16, 1/pm->gammas[i], pm->gammas[j],
         PNG_MAX_GAMMA_8, speed, pm->use_input_precision_16to8, 1/*strip16*/);
      if (fail(pm)) return;
   }
}

static void
perform_gamma_test(png_modifier *pm, int speed, int summary)
{
   /* First some arbitrary no-transform tests: */
   if (!speed)
   {
      perform_gamma_threshold_tests(pm);
      if (fail(pm)) return;
   }

   /* Now some real transforms. */
   perform_gamma_transform_tests(pm, speed);
   if (summary)
   {
      printf("Gamma correction error summary (output value error):\n");
      printf("  2 bit gray:  %.5f\n", pm->error_gray_2);
      printf("  4 bit gray:  %.5f\n", pm->error_gray_4);
      printf("  8 bit gray:  %.5f\n", pm->error_gray_8);
      printf(" 16 bit gray:  %.5f\n", pm->error_gray_16);
      printf("  8 bit color: %.5f\n", pm->error_color_8);
      printf(" 16 bit color: %.5f\n", pm->error_color_16);
   }

   /* The sbit tests produce much larger errors: */
   pm->error_gray_2 = pm->error_gray_4 = pm->error_gray_8 = pm->error_gray_16 =
   pm->error_color_8 = pm->error_color_16 = 0;
   perform_gamma_sbit_tests(pm, speed);
   if (summary)
   {
      printf("Gamma correction with sBIT:\n");
      if (pm->sbitlow < 8)
      {
         printf("  2 bit gray:  %.5f\n", pm->error_gray_2);
         printf("  4 bit gray:  %.5f\n", pm->error_gray_4);
         printf("  8 bit gray:  %.5f\n", pm->error_gray_8);
      }
      printf(" 16 bit gray:  %.5f\n", pm->error_gray_16);
      if (pm->sbitlow < 8)
         printf("  8 bit color: %.5f\n", pm->error_color_8);
      printf(" 16 bit color: %.5f\n", pm->error_color_16);
   }

   /* The 16 to 8 bit strip operations: */
   pm->error_gray_2 = pm->error_gray_4 = pm->error_gray_8 = pm->error_gray_16 =
   pm->error_color_8 = pm->error_color_16 = 0;
   perform_gamma_strip16_tests(pm, speed);
   if (summary)
   {
      printf("Gamma correction with 16 to 8 bit reduction:\n");
      printf(" 16 bit gray:  %.5f\n", pm->error_gray_16);
      printf(" 16 bit color: %.5f\n", pm->error_color_16);
   }
}

/* main program */
int main(int argc, const char **argv)
{
   int summary = 1; /* Print the error sumamry at the end */
   int speed = 0;   /* Speed test only (for gamma stuff) */

   /* This is an array of standard gamma values (believe it or not I've seen
    * every one of these mentioned somewhere.)
    *
    * In the following list the most useful values are first!
    */
   static double gammas[]={2.2, 1.0, 2.2/1.45, 1.8, 1.5, 2.4, 2.5, 2.62, 2.9};

   png_modifier pm;
   modifier_init(&pm);

   /* Default to error on warning: */
   pm.this.treat_warnings_as_errors = 1;

   /* Store the test gammas */
   pm.gammas = gammas;
   pm.ngammas = 3; /* for speed */
   pm.sbitlow = 8; /* because libpng doesn't do sbit below 8! */
   pm.use_input_precision_16to8 = 1; /* Because of the way libpng does it */

   /* Some default values (set the behavior for 'make check' here) */
   pm.maxout8 = .1;     /* Arithmetic error in *encoded* value */
   pm.maxabs8 = .00005; /* 1/20000 */
   pm.maxpc8 = .499;    /* I.e. .499% fractional error */
   pm.maxout16 = .499;  /* Error in *encoded* value */
   pm.maxabs16 = .00005;/* 1/20000 */
   /* NOTE: this is a reasonable perceptual limit, we assume that humans can
    * perceive light level differences of 1% over a 100:1 range, so we need to
    * maintain 1 in 10000 accuracy (in linear light space), this is what the
    * following guarantees.  It also allows significantly higher errors at
    * higher 16 bit values, which is important for performance.  The actual
    * maximum 16 bit error is about +/-1.9 in the fixed point implementation but
    * this is only allowed for values >38149 by the following:
    */
   pm.maxpc16 = .005;   /* I.e. 1/200% - 1/20000 */

   /* Now parse the command line options. */
   while (--argc >= 1)
      if (strcmp(*++argv, "-v") == 0)
         pm.this.verbose = 1;
      else if (strcmp(*argv, "-l") == 0)
         pm.log = 1;
      else if (strcmp(*argv, "-q") == 0)
         pm.this.verbose = pm.log = summary = 0;
      else if (strcmp(*argv, "-g") == 0)
         pm.ngammas = (sizeof gammas)/(sizeof gammas[0]);
      else if (strcmp(*argv, "-w") == 0)
         pm.this.treat_warnings_as_errors = 0;
      else if (strcmp(*argv, "-speed") == 0)
         speed = 1, pm.ngammas = (sizeof gammas)/(sizeof gammas[0]);
      else if (argc >= 1 && strcmp(*argv, "-sbitlow") == 0)
         --argc, pm.sbitlow = atol(*++argv);
      else if (argc >= 1 && strncmp(*argv, "-max", 4) == 0)
      {
         --argc;
         if (strcmp(4+*argv, "abs8") == 0)
            pm.maxabs8 = atof(*++argv);
         else if (strcmp(4+*argv, "abs16") == 0)
            pm.maxabs16 = atof(*++argv);
         else if (strcmp(4+*argv, "out8") == 0)
            pm.maxout8 = atof(*++argv);
         else if (strcmp(4+*argv, "out16") == 0)
            pm.maxout16 = atof(*++argv);
         else if (strcmp(4+*argv, "pc8") == 0)
            pm.maxpc8 = atof(*++argv);
         else if (strcmp(4+*argv, "pc16") == 0)
            pm.maxpc16 = atof(*++argv);
         else
         {
            fprintf(stderr, "pngvalid: %s: unknown 'max' option\n", *argv);
            exit(1);
         }
      }
      else
      {
         fprintf(stderr, "pngvalid: %s: unknown argument\n", *argv);
         exit(1);
      }

   /* Make useful base images */
   make_standard_images(&pm.this);
   make_gamma_images(&pm.this);

   /* Perform the standard and gamma tests. */
   if (!speed)
      perform_standard_test(&pm);
   perform_gamma_test(&pm, speed, summary && !speed);
   if (summary && !speed)
      printf("Results using %s point arithmetic %s\n",
#if defined(PNG_FLOATING_ARITHMETIC_SUPPORTED) || PNG_LIBPNG_VER < 10500
         "floating",
#else
         "fixed",
#endif
         (pm.this.nerrors || pm.this.treat_warnings_as_errors &&
            pm.this.nwarnings) ? "(errors)" : (pm.this.nwarnings ?
               "(warnings)" : "(no errors or warnings)")
      );

   /* Error exit if there are any errors, and maybe if there are any
    * warnings.
    */
   if (pm.this.nerrors || pm.this.treat_warnings_as_errors && pm.this.nwarnings)
   {
      if (!pm.this.verbose)
         fprintf(stderr, "pngvalid: %s\n", pm.this.error);
      fprintf(stderr, "pngvalid: %d errors, %d warnings\n", pm.this.nerrors,
         pm.this.nwarnings);
      exit(1);
   }

   return 0;
}

/* vim: set sw=3 ts=8 tw=80: */
