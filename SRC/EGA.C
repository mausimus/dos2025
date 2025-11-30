/* Quid Pro Quo
 * A deduction mini-game by mausimus and joker for DOSember Game Jam 2025
 * https://github.com/mausimus/dos2025
 * MIT License
 */

#include <DOS.H>
#include <STDIO.H>
#include <STDLIB.H>
#include <STRING.H>

#include "EGA.H"
#include "IO.H"
#include "UTIL.H"

#define SC_INDEX 0x3C4
#define SC_DATA 0x3C5
#define GC_INDEX 0x3CE
#define GC_DATA 0x3CF
#define FC_IS 0x3DA

#define TEXT_MEM ((uint far *)MK_FP(0xB800, 0x0000))

static uchar org_mode = 3;

void ega_vsync() {
  while ((inportb(FC_IS) & 0x08) == 0x08);
  while ((inportb(FC_IS) & 0x08) == 0x00);
}

void ega_set_plane_mask(uchar p) {
  outport(SC_INDEX, (p << 8) | 0x02);
}

void ega_set_write_plane(uchar p) {
  ega_set_plane_mask(1 << p);
}

void ega_set_read_plane(uchar p) {
  outport(GC_INDEX, (p << 8) | 0x04);
}

void ega_set_bit_mask(uchar b) {
  outport(GC_INDEX, (b << 8) | 0x08);
}

void ega_set_color(uchar col, uchar val) {
  asm {
    mov ax, 0x1000
    mov bl, col
    mov bh, val
    int 0x10
  }
}

void ega_set_palette(uchar *pal) {
  asm {
    mov ax, 0x1002
    mov bx, ds
    mov es, bx
    mov dx, [pal]
    mov bh, 0
    int 0x10
  }
}

void ega_graphics_mode() {
  uchar memory;
  asm {
    mov ah, 0x12
    mov bl, 0x10
    int 0x10
    mov memory, bl
  }
  if (memory > 4) {
    fatal_error("EGA video card not found!\n");
  }
  if (memory < 3) {
    fatal_error("Not enough video RAM, a 256K card is required!\n");
  }
  asm {
    mov ax, 0x0f00
    int 0x10
    mov org_mode, al
    mov ax, 0x000e
    int 0x10
  }
}

void ega_text_mode() {
  asm {
    mov al, org_mode
    mov ah, 0
    int 0x10
  }
  ega_set_active_page(0);
  io_load("END.BIN", 1920, TEXT_MEM);
  asm {
    mov ah, 2
    mov bh, 0
    mov dh, 13
    mov dl, 0
    int 0x10
  }
}

void ega_set_reset(uchar mask, uchar val) {
  outport(GC_INDEX, (val << 8) | 0x0);
  outport(GC_INDEX, (mask << 8) | 0x1);
}

void ega_set_active_page(uchar p) {
  asm {
    mov al, p
    mov ah, 5
    int 0x10
  }
}
