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

#include "ASSETS.H"
#include "AUDIO.H"
#include "COMMON.H"
#include "EGA.H"
#include "IO.H"
#include "SCRATCH.H"
#include "UTIL.H"
#include "VIDEO.H"

#define FADE_DELAY 1

uchar page_no;
uchar second_passed;

static int prev_time;
static uchar far *font;
static const uchar edge_loop[8] = {2, 3, 4, 6, 5, 4, 3, 2};

static uchar palettes[4][17] = {
  {0,1,2,3,4,5,6,7,16,17,18,19,20,21,22,23,0},
  {0,1,2,3,4,5,6,16,0,1,2,3,4,5,6,7,0},
  {0,0,1,1,1,1,1,0,0,0,1,1,1,1,1,16,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};

static uchar side_mask[2][7] = {{3, 7, 7, 7, 7, 7, 3},
                                {192, 224, 224, 224, 224, 224, 192}};

#include "BITMASKS.INC"

void internal_putimage(uint left, uint top, void far *bitmap, uchar page);

void video_active_page(uchar p) {
  page_no = p;
  ega_set_active_page(p);
}

void video_tick_frame() {
  // is actually half-second
  if (game_time != prev_time) {
    prev_time = game_time;
    second_passed = 1;
  } else {
    second_passed = 0;
  }
}

void video_vsync() {
  ega_vsync();
}

void load_font() {
  font = get_mem(256 * 9 * 2);
  io_load("FONT.DAT", 256 * 9 * 2, font);
}

void video_init() {
  ega_graphics_mode();
  load_font();
}

void video_end() {
  video_clear(0, 0);
  video_clear(1, 0);
  video_clear(2, 0);
  video_clear(3, 0);

  ega_text_mode();
}

void video_hotspot_blit(uint sx, uint sy, uint w, uint h, uint dx, uint dy,
                        uchar from_page, uchar to_page, uchar frame) {
  uint x, y, p, lineskip, mi;

  uchar far *from = PAGE_MEM(from_page);
  uchar far *to = PAGE_MEM(to_page);
  uchar far *fp;
  uchar far *tp;

  uchar eye_px;
  uchar dum_px;

  uint from_ofs = PAGE_OFS(sx, sy);
  uint to_ofs = PAGE_OFS(dx, dy);
  uint bytew = w / 8;

  uchar *bit_mask = bitmasks[frame];

  ASSERT(w % 8 == 0);
  ASSERT(sx % 8 == 0);
  ASSERT(dx % 8 == 0);

  lineskip = 80 - bytew;
  for (p = 0; p < 4; p++) {
    fp = from + from_ofs;
    tp = to + to_ofs;
    ega_set_write_plane(p);
    ega_set_read_plane(p);
    mi = 0;
    for (y = 0; y < h; y++) {
      for (x = 0; x < bytew; x++) {
        eye_px = *fp++;
        // latch video memory
        dum_px = *tp;
        // set write bit mask (transparency)
        ega_set_bit_mask(bit_mask[mi++]);
        *tp++ = eye_px;
      }
      fp += lineskip;
      tp += lineskip;
    }
  }
  ega_set_bit_mask(0xFF);
  ega_set_plane_mask(0xF);
}

void video_draw_side(uchar page_no, uint x, uint y, uchar bg, uchar side) {
  uint p, mi;

  uchar far *to = PAGE_MEM(page_no);
  uint to_ofs = PAGE_OFS(x, y + 1);
  uchar huge *tp;
  uchar eye_px;
  uchar dum_px;
  uchar bit;

  ASSERT(x % 8 == 0);

  for (p = 0; p < 4; p++) {
    tp = to + to_ofs;
    ega_set_write_plane(p);
    ega_set_read_plane(p);
    mi = 0;
    bit = bg & (1 << p) ? 0xFF : 0;
    for (y = 0; y < 7; y++) {
      eye_px = bit;
      // latch video memory
      dum_px = *tp;
      // set write bit mask (transparency)
      ega_set_bit_mask(side_mask[side][mi++]);
      *tp = eye_px;
      tp += 80;
    }
  }
  ega_set_bit_mask(0xFF);
  ega_set_plane_mask(0xF);
}

void internal_putimage(uint left, uint top, void far *bitmap, uchar page) {
  uint w = *((int far *)bitmap);
  uint h = *(((int far *)bitmap) + 1);
  video_bitmap_blit(bitmap, page, 0, 0, w, h, left, top);
}

void video_push_popup(uint x, uint y, uint w, uint h) {
  scratch_push(x, y, w, h);
}

void video_draw_edge(uchar page_no, uint x, uint y, uint w, uint h, uchar fg,
                     uchar bg, STYLE style) {
  char edge[80];
  uint l = w / 8;
  uchar chars = style == STYLE_HAND ? 0xf0 : 0xe0;
  if (style == STYLE_JAGGED) {
    chars = 0xd0;
    edge[l] = 0;
    while (l-- > 0) edge[l] = chars + 0x2e + (l % 8) / 4;
    video_text(page_no, edge, x, y, fg, bg);
    l = w / 8;
    while (l-- > 0) edge[l] = chars + 0x2c + (l % 8) / 4;
    video_text(page_no, edge, x, y + h - LINE_HEIGHT, fg, bg);
    edge[1] = 0;
    for (l = 0; l < h - LINE_HEIGHT; l += LINE_HEIGHT) {
      edge[0] = chars + edge_loop[l % 8];
      video_text(page_no, edge, x, y + l, fg, bg);
    }
    video_text(page_no, edge, x, y + h - LINE_HEIGHT, fg, bg);
    for (l = 0; l < h - LINE_HEIGHT; l += LINE_HEIGHT) {
      edge[0] = chars + 9 + edge_loop[l % 8];
      video_text(page_no, edge, x + w - 8, y + l, fg, bg);
    }
    video_text(page_no, edge, x + w - 8, y + h - LINE_HEIGHT, fg, bg);
  } else {
    memset(edge, chars + 1, l);
    edge[0] = chars;
    edge[l - 1] = chars + 2;
    edge[l] = 0;
    video_text(page_no, edge, x, y, fg, bg);
    memset(edge, chars + 6, l);
    edge[0] = chars + 5;
    edge[l - 1] = chars + 7;
    video_text(page_no, edge, x, y + h - LINE_HEIGHT, fg, bg);
    edge[0] = chars + 3;
    edge[1] = 0;
    for (l = LINE_HEIGHT; l < h - LINE_HEIGHT; l += LINE_HEIGHT)
      video_text(page_no, edge, x, y + l, fg, bg);
    edge[0] = chars + 4;
    for (l = LINE_HEIGHT; l < h - LINE_HEIGHT; l += LINE_HEIGHT)
      video_text(page_no, edge, x + w - 8, y + l, fg, bg);
  }
}

void video_show_view(VIEW *view) {
  POPUP *popup = &view->popup;
  uint margin = 0;

  if (popup->filename) {
    if (!popup->data)
      fatal_error("Popup not loaded!");

    if (popup->style & STYLE_BORDERLESS) {
      video_push_popup(popup->x, popup->y, popup->w, popup->h);
    } else {
      video_push_popup(popup->x - 16, popup->y - LINE_HEIGHT, popup->w + 32,
                       popup->h + 2 * LINE_HEIGHT);
      video_draw_edge(0, popup->x - 16, popup->y - LINE_HEIGHT, popup->w + 32,
                      popup->h + 2 * LINE_HEIGHT, 7, 8, STYLE_HAND);
      video_fill(0, popup->x - 8, popup->y, 8, popup->h, 8, 8);
      video_fill(0, popup->x + popup->w, popup->y, 8, popup->h, 8, 8);
    }
    internal_putimage(popup->x, popup->y, popup->data, page_no);
  } else {
    CLUE *clue = popup->clues;
    uchar fg = POPUP_FG, bg = POPUP_BG, eg = POPUP_EG;

    switch (popup->style) {
      case STYLE_HAND:
      case STYLE_JAGGED:
        fg = POPUP_HAND_FG;
        bg = POPUP_HAND_BG;
        eg = POPUP_HAND_EG;
        break;
      case STYLE_PRINT:
        fg = POPUP_PRINT_FG;
        bg = POPUP_PRINT_BG;
        eg = POPUP_PRINT_EG;
        break;
      default:
        break;
    }

    if (view->id == VIEW_INVENTORY) {
      fg = INV_FG;
      bg = INV_BG;
      eg = INV_EG;
    }

    if (popup->style == STYLE_JAGGED) {
      video_push_popup(popup->x - 8, popup->y - LINE_HEIGHT, popup->w + 16,
                       popup->h + 2 * LINE_HEIGHT);
    } else {
      video_push_popup(popup->x, popup->y, popup->w, popup->h);
    }

    if (popup->w && !(popup->style & STYLE_BORDERLESS)) {
      video_fill(0, popup->x + 8, popup->y + LINE_HEIGHT, popup->w - 16,
                 popup->h - 2 * LINE_HEIGHT, bg, bg);
      video_draw_edge(0, popup->x, popup->y, popup->w, popup->h, eg, bg,
                      popup->style);
    } else {
      video_fill(0, popup->x, popup->y, popup->w, popup->h, bg, bg);
    }

    if (popup->style == STYLE_JAGGED) {
      video_draw_edge(0, popup->x - 8, popup->y - LINE_HEIGHT, popup->w + 16,
                      popup->h + 2 * LINE_HEIGHT, 7, 8, STYLE_HAND);
    }

    if (view->id == VIEW_INVENTORY)
      margin = INV_MARGIN;

    video_text(0, popup->message, popup->x + 16 + margin,
               popup->y + LINE_HEIGHT, fg, bg);
    // render clues?
    while (clue) {
      video_fill(0, clue->x, clue->y + LINE_HEIGHT, clue->w, 1, 4, bg);
      video_fill(0, clue->x, clue->y + LINE_HEIGHT + 1, clue->w, 1, bg, 4);
      clue = clue->next;
    }
  }
}

void video_load_image(const char *file_name, uchar page, uint x, uint y,
                      uint size) {
  void far *p;

  p = (void far *)get_mem(size);
  if (p == NULL)
    fatal_error("Not enough memory!");
  io_load(file_name, size, p);
  internal_putimage(x, y, p, page);
  free_mem(p, size);
}

void video_close_popup() {
  scratch_pop();
}

void video_draw_debug() {
  char tstr[10];
  ulong fcl = farcoreleft();
  sprintf(tstr, "%lu", fcl);
  video_text(0, tstr, 0, 0, 15, 0);
}

uchar video_get_draw_frame(SPRITE *sprite) {
  uchar draw_frame = sprite->frame;
  if (draw_frame >= sprite->frames)  // loop back
  {
    uchar how_many_back = draw_frame - sprite->frames + 1;
    uchar last_frame = sprite->frames - 1;
    draw_frame = last_frame - how_many_back;
  }
  return draw_frame;
}

void video_draw_hotspot_at(uint x, uint y, SPRITE *sprite) {
  uchar draw_frame = video_get_draw_frame(sprite);
  video_hotspot_blit(sprite->sx + (sprite->w * draw_frame), sprite->sy,
                     sprite->w, sprite->h, x, y, SPRITE_PAGE, 0, draw_frame);
}

void video_draw_sprite(SPRITE *sprite) {
  uchar draw_frame = video_get_draw_frame(sprite);

  if (sprite->data == NULL) {
    if (sprite->text) {
      video_text(0, sprite->text, sprite->sx, sprite->sy, INV_FG, POPUP_BG);
    } else
      fatal_error("Sprite not loaded!");
  } else {
    video_bitmap_blit(sprite->data, 0, sprite->w * draw_frame, 0, sprite->w,
                      sprite->h, sprite->sx, sprite->sy);
  }
}

// fast VRAM-to-VRAM copy using all four EGA latches in parallel
// we have to use movsb since latches are byte-sized only
// bytew - width of a line in bytes (pixels/8)
// from_skip/to_skip - bytes to skip to get to next line
void video_vram_blit(uchar from_page, uchar to_page, uint from_ofs,
                     uint to_ofs, uint bytew, uint h, uint from_skip,
                     uint to_skip) {
  uint from_seg = VID_MEM + (PAGE_SIZE * from_page);
  uint to_seg = VID_MEM + (PAGE_SIZE * to_page);

  ega_set_plane_mask(0xFF);
  ega_set_bit_mask(0);
  asm {
    push ds
    push es
    mov ax, from_seg
    mov ds, ax
    mov ax, to_seg
    mov es, ax
    mov si, from_ofs
    mov di, to_ofs
    mov bx, from_skip
    mov dx, to_skip
    cld
  }
line:
  asm {
    mov cx, bytew
    rep movsb
    add si, bx
    add di, dx
    dec h
    jnz short line
    pop es
    pop ds
  }
  ega_set_bit_mask(0xFF);
}

// bitmap to VRAM copy
void video_bitmap_blit(uchar far *bitmap, uchar to_page, uint sx, uint sy,
                       uint w, uint h, uint dx, uint dy) {
  uint bmw, bytew;  // bitmap width
  uchar far *fp = PAGE_MEM(to_page);
  uchar far *bm = (uchar far *)bitmap;
  uint bmw8;
  uint lineskip;

  bmw = *((int far *)bitmap);  // whole bitmap width
  bytew = w / 8;               // byte width to copy
  bm += 4;                     // skip header
  bm += (sx / 8) + sy * bmw / 2;
  bmw8 = (bmw / 8) - bytew;
  lineskip = 80 - bytew;

  ASSERT(w % 8 == 0);
  ASSERT(sx % 8 == 0);
  ASSERT(bmw % 8 == 0);

  // v1 - line by line
  fp += (dx / 8) + dy * 80;

  asm {        
    push es
    push ds
    lds si, bm
    les di, fp

    mov bx, h
    mov ax, 0x2
    mov dx, 0x3c4
  }
copy_line:
  asm {
    mov ah, 0x01
    out dx, ax
    mov cx, bytew
    rep movsb
    add si, bmw8
    sub di, bytew

    mov ah, 0x02
    out dx, ax
    mov cx, bytew
    rep movsb
    add si, bmw8
    sub di, bytew

    mov ah, 0x04
    out dx, ax
    mov cx, bytew
    rep movsb
    add si, bmw8
    sub di, bytew

    mov ah, 0x08
    out dx, ax
    mov cx, bytew
    rep movsb
    add si, bmw8

    add di, lineskip
    dec bx
    jnz short copy_line
    pop ds
    pop es
  }
  ega_set_plane_mask(0xF);
}

void video_copy_image(uint sx, uint sy, uint w, uint h, uint dx, uint dy,
                      uchar from_page, uchar to_page) {
  uint from_ofs = PAGE_OFS(sx, sy);
  uint to_ofs = PAGE_OFS(dx, dy);
  uint bytew = w / 8;
  uint lineskip = 80 - bytew;

  ASSERT(w % 8 == 0);

  video_vram_blit(from_page, to_page, from_ofs, to_ofs, bytew, h, lineskip,
                  lineskip);
}

static void video_lines(const char *c, uchar far *s, uchar inv) {
  uint from_seg = NORM_SEG(font);
  uint from_ofs = NORM_OFF(font);
  uint text_seg = NORM_SEG(c);
  uint text_ofs = NORM_OFF(c);
  uint to_seg = FP_SEG(s);
  uint to_ofs = FP_OFF(s);
  uchar fill = 0;

  ega_set_plane_mask((~inv) & 0xF);

draw_text:    
  asm {
    push ds
    push es
    mov bx, text_ofs
    xor dx, dx
    mov ax, from_seg
    mov ds, ax
    cld
  }
next_char:
  asm {    
    mov ax, text_seg
    mov es, ax
    mov di, bx
    mov cl, [es:di]

    cmp cl, 10
    jbe short low_char

    // add character offset to font pointer
    xor ch, ch
    mov ax, cx
    shl ax, 3
    add ax, cx
    mov si, from_ofs
    add si, ax

    mov ax, to_seg
    mov es, ax
    mov di, to_ofs
    add di, dx

    inc bx
    inc dx

    // start
    movsb
    add di, 79
    movsb
    add di, 79
    movsb
    add di, 79
    movsb
    add di, 79
    movsb
    add di, 79
    movsb
    add di, 79
    movsb
    add di, 79
    movsb
    add di, 79
    movsb
    add di, 79

    jmp short next_char
  }
low_char:
  asm {
    test cl, cl
    jz short text_done
    cmp cl, 10
    je short line_done
    inc bx
    jmp short next_char
  }
line_done:
  asm {
    add to_ofs, (LINE_HEIGHT * 80)
    xor dx, dx
    inc bx
    jmp short next_char
  }
text_done:
  asm {
    pop es
    pop ds
  }
  if (inv) {
    ega_set_plane_mask(inv);
    inv = 0;
    fill = 0xff;
    from_ofs += 2304;  // inverted font base
    to_ofs = FP_OFF(s);
    goto draw_text;
  }
}

void video_text(uchar page, const char *t, uint x, uint y, uchar fg,
                uchar bg) {
  uchar far *s = PAGE_MEM_OFS(page, x, y);
  uchar mask = 0, val = 0, inv = 0;

  ASSERT(x % 8 == 0);

  /*
  each bitplane:
  (a) always 1 <=> fg and bg have bit set <=> set in mask, set val 1
  (b) always 0 <=> fg and bg have bit clear <=> set in mask, set val 0
  (c) 1 if font, 0 if not <=> fg has set, bg has clear <=> do not set in mask,
  copy actual (d) 0 if font, 1 if not <=> bg has set, fg has clear <=> do not
  set in mask, copy inverse
  */

  val = fg & bg;
  inv = (~fg) & bg;
  mask = (~fg) ^ bg;

  ega_set_reset(mask, val);
  video_lines(t, s, inv);
  ega_set_reset(0, 0);
  ega_set_plane_mask(0xF);
}

void video_load_sprite(SPRITE *sprite) {
  uint size = IMG_SIZE(sprite->w * sprite->frames, sprite->h);
  if (sprite->frames == 0)
    return;
  sprite->data = (uchar far *)get_mem(size);
  io_load(sprite->filename, size, sprite->data);
}

// fast VRAM fill by preloading EGA latches with destination color
// and writing latched values back to the whole area
void video_fill(uchar page, uint x, uint y, uint w, uint h, uchar c1,
                uchar c2) {
  uchar far *tmp = PAGE_FOOT(0);
  uint to_seg = VID_MEM + (PAGE_SIZE * page);
  uint to_ofs = PAGE_OFS(x, y);
  uint stepw = w / 16;
  uint to_skip = 80 - (w / 8);  // in bytes
  uchar p, leadb = (x & 0xF), trailb = (x + w) & 0xF,
           doubleb = leadb & trailb;

  ASSERT(w % 8 == 0);
  ASSERT(x % 8 == 0);

  // set 8 pixels in a temporary location
  if (c1 == c2) {
    for (p = 0; p < 4; p++) {
      ega_set_write_plane(p);
      if (c1 & (1 << p))
        *tmp = 0xFF;
      else
        *tmp = 0x00;
    }
  } else {
    for (p = 0; p < 4; p++) {
      int b1 = c1 & (1 << p);
      int b2 = c2 & (1 << p);
      ega_set_write_plane(p);
      if (b1 && b2)
        *tmp = 0xFF;
      else if (b1)
        *tmp = 0xAA;
      else if (b2)
        *tmp = 0x55;
      else
        *tmp = 0x00;
    }
  }

  // populate latches
  ega_set_plane_mask(0xFF);
  ega_set_bit_mask(0);
  p = *tmp;

  if (doubleb)
    stepw--;

  if (to_skip == 0) {
    // continuous - single pass
    stepw *= h;
    h = 1;
  }

  asm {
    push es
    mov ax, to_seg
    mov es, ax
    mov di, to_ofs
    cld
    cmp doubleb, 0
    jne line_db
    cmp trailb, 0
    jne line_tb
    cmp leadb, 0
    jne line_lb
  }
line_w:
  asm {
    mov cx, stepw
    rep stosw
    add di, to_skip
    dec h
    jnz line_w
    pop es
    jmp line_done
  }
line_lb:
  asm {
    stosb
    mov cx, stepw
    rep stosw
    add di, to_skip
    dec h
    jnz line_lb
    pop es
    jmp line_done
  }
line_tb:
  asm {
    mov cx, stepw
    rep stosw
    stosb
    add di, to_skip
    dec h
    jnz line_tb
    pop es
    jmp line_done
  }
line_db:
  asm {
    stosb
    mov cx, stepw
    rep stosw
    stosb
    add di, to_skip
    dec h
    jnz line_db
    pop es
    jmp line_done
  }
line_done:
  ega_set_bit_mask(0xFF);
}

void video_clear(uchar page, uchar c) {
  video_fill(page, 0, 0, 640, 200, c, c);
}

void video_fade_out() {
  uchar s;
  for (s = 1; s < 4; s++) {
    timer_start(FADE_DELAY);
    ega_set_palette(palettes[s]);
    timer_wait();
  }
}

void video_fade_in() {
  uchar s;
  for (s = 2; s > 0; s--) {
    timer_start(FADE_DELAY);
    ega_set_palette(palettes[s]);
    timer_wait();
  }
  ega_set_palette(palettes[0]);
}
