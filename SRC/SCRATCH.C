/* Quid Pro Quo
 * A deduction mini-game by mausimus and joker for DOSember Game Jam 2025
 * https://github.com/mausimus/dos2025
 * MIT License
 */

#include <STDIO.H>

#include "COMMON.H"
#include "SCRATCH.H"
#include "UTIL.H"
#include "VIDEO.H"

#define SCRATCH_MAX 5
#define SCRATCH_MEM 65535l
#define SPRITES_W 16
#define SPRITES_H 8
#define SPRITES_Y 40

uchar scratch_num = 0;
static uint scratch_ofs = 0;
static uint sprites_y = SPRITES_Y;

typedef struct {
  uint x;
  uint y;
  uint w;
  uint h;
  uint ofs;
} SCRATCH;

SCRATCH scratches[SCRATCH_MAX];

void scratch_push(uint x, uint y, uint w, uint h) {
  SCRATCH *s;
  uint len = (w / 8) * h;
  uint from_ofs = y * 80 + (x / 8);
  uint bytew = w / 8;
  uint lineskip = 80 - bytew;

  if (scratch_num == SCRATCH_MAX - 1)
    fatal_error("No more scratches allowed!");

  if ((long)scratch_ofs + (long)len > SCRATCH_MEM)
    fatal_error("No more scratch memory!");

  s = scratches + scratch_num;
  s->x = x;
  s->y = y;
  s->w = w;
  s->h = h;
  video_vram_blit(0, SCRATCH_PAGE, from_ofs, scratch_ofs, bytew, h, lineskip,
                  0);
  scratch_ofs += len;
  scratch_num++;
}

void scratch_pop() {
  SCRATCH *s;
  uint len;
  uint to_ofs;
  uint bytew;
  uint lineskip;

  if (!scratch_num)
    fatal_error("No scratch to pop!");

  scratch_num--;
  s = scratches + scratch_num;
  len = (s->w / 8) * s->h;
  to_ofs = s->y * 80 + (s->x / 8);
  bytew = s->w / 8;
  lineskip = 80 - bytew;
  scratch_ofs -= len;
  video_vram_blit(SCRATCH_PAGE, 0, scratch_ofs, to_ofs, bytew, s->h, 0,
                  lineskip);
}

void hotspot_pop(uint n) {
  sprites_y -= n * SPRITES_H;
  if (sprites_y < SPRITES_Y) {
    char msg[20];
    sprintf(msg, "%d", n);
    fatal_errorf("Popped empty sprite attempting %s pops!", msg);
  }
}

void hotspot_clear() {
  sprites_y = SPRITES_Y;
}

uint hotspot_push(uint sx, uint sy) {
  int i;
  uint y = sprites_y;

  // create frames
  for (i = 0; i < 3; i++) {
    video_copy_image(sx, sy, SPRITES_W, SPRITES_H, i * SPRITES_W, y, 0,
                     SPRITE_PAGE);
    video_hotspot_blit(i * SPRITES_W, 32, SPRITES_W, SPRITES_H, i * SPRITES_W,
                       y, SPRITE_PAGE, SPRITE_PAGE, i);
  }
  sprites_y += SPRITES_H;
  return y;
}

void hotspot_draw(uint dx, uint dy, uint sy, int frame) {
  video_copy_image(SPRITES_W * frame, sy, SPRITES_W, SPRITES_H, dx, dy,
                   SPRITE_PAGE, 0);
}
