/* Quid Pro Quo
 * A deduction mini-game by mausimus and joker for DOSember Game Jam 2025
 * https://github.com/mausimus/dos2025
 * MIT License
 */

#if !defined(UTIL_H)
#define UTIL_H

#ifdef DEBUG
#define ASSERT(x) assert(x)
#else
#define ASSERT(x) ;
#endif

#define NORM_SEG(f) (FP_SEG(f) + (FP_OFF(f) >> 4))
#define NORM_OFF(f) (FP_OFF(f) & 0xF)

#include "COMMON.H"

const char *stristr(const char *text, const char *string);
void far *get_mem(uint size);
void free_mem(void far *p, uint size);
void dump_mem();
void check_mem();
void fatal_error(const char *msg);
void fatal_errorf(const char *msg, const char *arg);
void assert(int v);

#endif
