/* Quid Pro Quo
 * A deduction mini-game by mausimus and joker for DOSember Game Jam 2025
 * https://github.com/mausimus/dos2025
 * MIT License
 */

#include <ALLOC.H>
#include <CTYPE.H>
#include <DOS.H>
#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>

#include "AUDIO.H"
#include "IO.H"
#include "UTIL.H"
#include "VIDEO.H"

static ulong alloc_mem = 0;
static ulong max_mem = 0;
static ulong freed_mem = 0;
static ulong starting_near = 0;
static ulong starting_far = 0;

const char *stristr(const char *text, const char *string) {
  char c;
  char s = toupper(*string);
  int l = strlen(string);
  while ((c = toupper(*text))) {
    if (c == s) {
      if (memicmp(text, string, l) == 0)
        return text;
    }
    text++;
  }
  return NULL;
}

void check_mem() {
  starting_near = coreleft();
  starting_far = farcoreleft();
}

void far *get_mem(uint size) {
  void far *p = (void far *)farmalloc(size);
  ulong curr_mem;
  if (p == NULL) {
    audio_end();
    video_end();
    printf("Out of memory! Tried to allocate %u bytes.\n", size);
    dump_mem();
    abort();
  }
  alloc_mem += size;
  curr_mem = alloc_mem - freed_mem;
  if (curr_mem > max_mem)
    max_mem = curr_mem;
  if (FP_OFF(p) > 15)
    fatal_error("Large offset!");
  return p;
}

void free_mem(void far *p, uint size) {
  farfree(p);
  freed_mem += size;
}

void dump_mem() {
#ifdef DEBUG
  printf("Starting Far :  %8lu\n", starting_far);
  printf("Starting Near:  %8lu\n", starting_near);
  printf("Allocated:      %8lu\n", alloc_mem);
  printf("Freed:          %8lu\n", freed_mem);
  printf("Current:        %8lu\n", alloc_mem - freed_mem);
  printf("Peak:           %8lu\n", max_mem);
#endif  
}

void fatal_errorf(const char *msg, const char *arg) {
  char full[128];
  sprintf(full, msg, arg);
  fatal_error(full);
}

void fatal_error(const char *msg) {
  audio_end();
  video_end();
  io_end();
  printf("FATAL ERROR: %s\n", msg);
  abort();
}

void assert(int v) {
  if (!v)
    fatal_error("Assertion failed");
}
