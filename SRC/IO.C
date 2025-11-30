/* Quid Pro Quo
 * A deduction mini-game by mausimus and joker for DOSember Game Jam 2025
 * https://github.com/mausimus/dos2025
 * MIT License
 */

#include <ALLOC.H>
#include <DOS.H>
#include <MEM.H>
#include <STDIO.H>
#include <STRING.H>

#include "IO.H"
#include "UTIL.H"

#define BUF_SIZE 65000u
#define RLE_MARKER 0x96

typedef struct PACKED_FILE {
  const char *filename;
  long offset;
  uint size;
} PACKED_FILE;

static FILE *pack_file;

static PACKED_FILE packed_files[] = {
#include "ASSETS.INC"
};

#define NUM_FILES (sizeof(packed_files) / sizeof(PACKED_FILE))

static void derle(uchar far *inp, uchar far *outp, uint clen) {
  // clang-format off
  asm {
    push es
    push ds
    lds si, inp
    les di, outp
    mov bx, si
		mov ax, clen
		add bx, ax
		xor ch, ch
		cld
  }
rle_loop:
  asm {
    cmp si, bx
    je short rle_done
    lodsb
    cmp al, RLE_MARKER
    jne short rle_none
    lodsb
    mov cl, al
    lodsb
    rep stosb
    jmp short rle_loop
  }
rle_none:
  asm {
    stosb
    jmp short rle_loop
  }
rle_done:
  asm {
    pop ds
    pop es
  }
  // clang-format on
}

uint io_len(const char *filename) {
  uchar i;
  for (i = 0; i < NUM_FILES; i++) {
    PACKED_FILE *pf = packed_files + i;
    if (strcmp(pf->filename, filename) == 0) {
      return pf->size;
    }
  }
  fatal_errorf("Unable to find file %s", filename);
  return 0;
}

void io_read(void far *p, uint size, FILE *f, uchar rle, uint rsize) {
  uint ofs = 0;
  if (rle) {
    uchar far *cp = (uchar far *)p;
    uint rstart = size - rsize;
    uchar far *cstart = cp + rstart;

    // load compressed data at the end
    _dos_read(f->fd, cstart, rsize, &ofs);

    // copy header to output
    *cp++ = *cstart++;
    *cp++ = *cstart++;
    *cp++ = *cstart++;
    *cp++ = *cstart++;

    derle(cstart, cp, rsize - 4);
  } else {
    _dos_read(f->fd, p, size, &ofs);
  }
}

void test_mem() {
  uint cl;
  ulong fcl;

  cl = coreleft();
  fcl = farcoreleft();
  printf("cl = %u, fcl = %lu\n", cl, fcl);
}

void io_load(const char *filename, uint size, void far *p) {
  uchar rle = 0;
  int skip = 0;
  uchar i;

  switch (filename[strlen(filename) - 3]) {
    case 'R':
      rle = 1;
      break;
    case 'S':
      skip = 36;
      break;
  }

  for (i = 0; i < NUM_FILES; i++) {
    PACKED_FILE *pf = packed_files + i;
    if (strcmp(pf->filename, filename) == 0) {
      fseek(pack_file, pf->offset + skip, SEEK_SET);
      io_read(p, size, pack_file, rle, pf->size - skip);
      return;
    }
  }
  fatal_errorf("Unable to open file %s", filename);
}

void io_dimensions(const char *filename, uint *w, uint *h) {
  uchar i;
  uint wh[2];
  for (i = 0; i < NUM_FILES; i++) {
    PACKED_FILE *pf = packed_files + i;
    if (strcmp(pf->filename, filename) == 0) {
      fseek(pack_file, pf->offset, SEEK_SET);
      fread((void *)wh, 2, 2, pack_file);
      *w = wh[0];
      *h = wh[1];
      return;
    }
  }
  fatal_errorf("Unable to find file %s", filename);
}

void io_init() {
  pack_file = fopen("QUIDPROQ.RVD", "rb");
  if (pack_file == NULL) {
    fatal_error("Unable to open assets file");
  }
}

void io_end() {
  if (pack_file) {
    fclose(pack_file);
  }
}