/* Quid Pro Quo
 * A deduction mini-game by mausimus and joker for DOSember Game Jam 2025
 * https://github.com/mausimus/dos2025
 * MIT License
 */

#if !defined(SCRATCH_H)
#define SCRATCH_H

#include "COMMON.H"

extern uchar scratch_num;
void scratch_push(uint x, uint y, uint w, uint h);
void scratch_pop();

uint hotspot_push(uint sx, uint sy);
void hotspot_pop(uint n);
void hotspot_clear();
void hotspot_draw(uint dx, uint dy, uint sy, int frame);

#endif
