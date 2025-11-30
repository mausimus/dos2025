/* Quid Pro Quo
 * A deduction mini-game by mausimus and joker for DOSember Game Jam 2025
 * https://github.com/mausimus/dos2025
 * MIT License
 */

#if !defined(IO_H)
#define IO_H

#include "COMMON.H"

void io_init();
void io_load(const char *filename, uint size, void far *p);
void io_dimensions(const char *filename, uint *w, uint *h);
uint io_len(const char* filename);
void test_mem();
void io_end();

#endif
