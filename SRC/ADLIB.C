/* Quid Pro Quo
 * A deduction mini-game by mausimus and joker for DOSember Game Jam 2025
 * https://github.com/mausimus/dos2025
 * MIT License
 */

#include <DOS.H>

#include "ADLIB.H"
#include "IO.H"

#define ADLIB_ADDRESS 0x388
#define ADLIB_DATA 0x389

void adlib_reset(void) {
  uchar i, slot1, slot2;
  int j;
  static uchar slots[9][2] = {{0, 3},  {1, 4},   {2, 5},   {6, 9},  {7, 10},
                              {8, 11}, {12, 15}, {13, 16}, {14, 17}};
  static uchar ofs[18] = {0,  1,  2,  3,  4,  5,  8,  9,  10,
                          11, 12, 13, 16, 17, 18, 19, 20, 21};

  adlib_out(1, 0x20);
  adlib_out(8, 0);
  adlib_out(0xBD, 0);

  for (i = 0; i < 9; i++) {
    slot1 = ofs[slots[i][0]];
    slot2 = ofs[slots[i][1]];

    adlib_out(0xB0 + i, 0);
    adlib_out(0xA0 + i, 0);
    adlib_out(0xE0 + slot1, 0);
    adlib_out(0xE0 + slot2, 0);
    adlib_out(0x60 + slot1, 0xff);
    adlib_out(0x60 + slot2, 0xff);
    adlib_out(0x80 + slot1, 0xff);
    adlib_out(0x80 + slot2, 0xff);
    adlib_out(0x40 + slot1, 0xff);
    adlib_out(0x40 + slot2, 0xff);
  }
  for (j = 0; j < 256; j++) {
    adlib_out(j, 0);
  }

  adlib_out(0x01, 0x20);
}

void adlib_stop(int v) {
  adlib_out(0xB0 + v, 0);
}

void adlib_play(int v, int freq, int octave) {
  adlib_out(0xA0 + v, freq & 0xff);
  adlib_out(0xB0 + v, (freq >> 8) | (octave << 2) | 0x20);
}

void adlib_set(int v, ADLIB_INST *ins) {
  static int regs[] = {0x20, 0x23, 0x40, 0x43, 0x60, 0x63,
                       0x80, 0x83, 0xe0, 0xe3, 0xc0};

  int ofs = v % 3 + ((v / 3) << 3);
  int i;

  for (i = 0; i < 10; i++) adlib_out(regs[i] + ofs, ins->regs[i]);

  adlib_out(0xc0 + v, ins->regs[10]);
}

void adlib_load(const char *filename, ADLIB_INST *ins) {
  io_load(filename, sizeof(ADLIB_INST), (void far *)ins);
}

void adlib_volume(int v, int vol) {
  int ofs = v % 3 + ((v / 3) << 3);
  adlib_out(0x43 + ofs, vol);
}

void adlib_out(uchar reg, uchar value) {
  asm {
    mov al, reg
    mov dx, ADLIB_ADDRESS
    out dx, al
    in al, dx
    in al, dx
    in al, dx
    in al, dx
    in al, dx
    in al, dx
    mov al, value
    mov dx, ADLIB_DATA
    out dx, al
    mov dx, ADLIB_ADDRESS
    in al, dx
    in al, dx
    in al, dx
    in al, dx
    in al, dx
    in al, dx
    in al, dx
    in al, dx
    in al, dx
    in al, dx
    in al, dx
    in al, dx
    in al, dx
    in al, dx
    in al, dx
    in al, dx
    in al, dx
    in al, dx
    in al, dx
    in al, dx
    in al, dx
    in al, dx
    in al, dx
    in al, dx
    in al, dx
    in al, dx
    in al, dx
    in al, dx
    in al, dx
    in al, dx
    in al, dx
    in al, dx
    in al, dx
    in al, dx
    in al, dx
  }
}
