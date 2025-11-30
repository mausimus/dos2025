/* Quid Pro Quo
 * A deduction mini-game by mausimus and joker for DOSember Game Jam 2025
 * https://github.com/mausimus/dos2025
 * MIT License
 */

#if !defined(VIDEO_H)
#define VIDEO_H

#include "ASSETS.H"
#include "COMMON.H"

extern uchar page_no;
extern uchar second_passed;

void video_init();
void video_end();
void video_tick_frame();
void video_draw_debug();
void video_active_page(uchar p);
void video_vsync();

void video_show_view(VIEW* view);
void video_push_popup(uint x, uint y, uint w, uint h);
void video_close_popup();
void video_load_image(const char *file_name, uchar page, uint x, uint y, uint size);
void video_copy_image(uint sx, uint sy, uint w, uint h, uint dx, uint dy,
                      uchar from_page, uchar to_page);
void video_vram_blit(uchar from_page, uchar to_page, uint from_ofs,
                     uint to_ofs, uint bytew, uint h, uint from_skip,
                     uint to_skip);
void video_bitmap_blit(uchar far *bitmap, uchar to_page, uint sx, uint sy,
                       uint w, uint h, uint dx, uint dy);
void video_load_sprite(SPRITE *sprite);
uchar video_get_draw_frame(SPRITE *sprite);

void video_draw_sprite(SPRITE *sprite);
void video_draw_hotspot_at(uint x, uint y, SPRITE *sprite);
void video_hotspot_blit(uint sx, uint sy, uint w, uint h, uint dx, uint dy,
                        uchar from_page, uchar to_page, uchar frame);
void video_text(uchar page, const char *t, uint x, uint y, uchar fg,
                uchar bg);
void video_fill(uchar page, uint x, uint y, uint w, uint h, uchar c1,
                uchar c2);
void video_draw_edge(uchar page_no, uint x, uint y, uint w, uint h, uchar fg,
                     uchar bg, STYLE style);
void video_draw_side(uchar page_no, uint x, uint y, uchar bg, uchar side);
void video_clear(uchar page, uchar c);
void video_fade_out();
void video_fade_in();

#endif
