/* Quid Pro Quo
 * A deduction mini-game by mausimus and joker for DOSember Game Jam 2025
 * https://github.com/mausimus/dos2025
 * MIT License
 */

#if !defined(EGA_H)
#define EGA_H

#include "COMMON.H"

#define VID_MEM 0xA000
#define PAGE_SIZE 0x0400
#define PAGE_MEM(p) ((uchar far *)MK_FP(VID_MEM + (PAGE_SIZE * p), 0x0000))
#define PAGE_OFS(x, y) (((x) >> 3) + (y) * 80)
#define PAGE_MEM_OFS(p, x, y) (PAGE_MEM(p) + PAGE_OFS(x, y))
#define PAGE_FOOT(p) (uchar far *)MK_FP(VID_MEM + (PAGE_SIZE * p), 0xFA00)

void ega_graphics_mode();
void ega_text_mode();
void ega_vsync();
void ega_set_plane_mask(uchar p);
void ega_set_bit_mask(uchar b);
void ega_set_write_plane(uchar p);
void ega_set_read_plane(uchar p);
void ega_set_reset(uchar mask, uchar val);
void ega_set_active_page(uchar p);
void ega_set_color(uchar col, uchar val);
void ega_set_palette(uchar *pal);

#endif
