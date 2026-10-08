/* LD_PRELOAD malloc interposer: every allocation is placed at the end of a
 * page whose following page is PROT_NONE, so ANY write past the requested
 * size segfaults. Detects heap OOB writes deterministically without ASan.
 */
#define _GNU_SOURCE
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

typedef struct { void *user; void *base; size_t size; int guard; } slot;
static slot slots[65536];
static int nslots = 0;

static void *guarded_malloc(size_t size)
{
   long page = sysconf(_SC_PAGESIZE);
   if (size == 0) size = 1;
   size_t chunk = (size + page - 1) / page * page;
   unsigned char *base = mmap(NULL, chunk + page, PROT_READ | PROT_WRITE,
       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
   if (base == MAP_FAILED) return NULL;
   if (mprotect(base + chunk, page, PROT_NONE) != 0) { munmap(base, chunk + page); return NULL; }
   void *user = base + chunk - size;
   if (nslots < 65536) { slots[nslots].user = user; slots[nslots].base = base; slots[nslots].size = size; nslots++; }
   return user;
}

void *malloc(size_t size) { return guarded_malloc(size); }

void *calloc(size_t n, size_t size)
{
   size_t total = n * size;
   void *p = guarded_malloc(total);
   if (p) memset(p, 0, total);
   return p;
}

void *realloc(void *ptr, size_t size)
{
   void *np = guarded_malloc(size);
   if (np && ptr)
   {
      int i;
      size_t old = 0;
      for (i = 0; i < nslots; i++) if (slots[i].user == ptr) { old = slots[i].size; break; }
      if (old > size) old = size;
      memcpy(np, ptr, old);
   }
   free(ptr);
   return np;
}

void free(void *ptr)
{
   if (ptr == NULL) return;
   int i;
   for (i = 0; i < nslots; i++)
   {
      if (slots[i].user == ptr)
      {
         munmap(slots[i].base, ((slots[i].size + sysconf(_SC_PAGESIZE) - 1) / sysconf(_SC_PAGESIZE)) * sysconf(_SC_PAGESIZE) + sysconf(_SC_PAGESIZE));
         slots[i].user = NULL;
         return;
      }
   }
   /* not tracked; fall back to a plain free */
   /* nothing to do - unmapped unknowns are leaks in this interposer */
}
