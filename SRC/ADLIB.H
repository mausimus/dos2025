/* Quid Pro Quo
 * A deduction mini-game by mausimus and joker for DOSember Game Jam 2025
 * https://github.com/mausimus/dos2025
 * MIT License
 */

#if !defined(ADLIB_H)
#define ADLIB_H

#include "COMMON.H"

typedef struct ADLIB_INST {
  uchar regs[11];
} ADLIB_INST;

void adlib_out(uchar reg, uchar value);
void adlib_reset();
void adlib_play(int v, int freq, int octave);
void adlib_stop(int v);
void adlib_volume(int v, int vol);
void adlib_set(int v, ADLIB_INST *ins);
void adlib_load(const char *filename, ADLIB_INST *ins);

#endif
