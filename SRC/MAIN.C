/* Quid Pro Quo
 * A deduction mini-game by mausimus and joker for DOSember Game Jam 2025
 * https://github.com/mausimus/dos2025
 * MIT License
 */

#include <ALLOC.H>
#include <CONIO.H>
#include <CTYPE.H>
#include <DOS.H>
#include <MEM.H>
#include <STDIO.H>
#include <STDLIB.H>

#include "ASSETS.H"
#include "AUDIO.H"
#include "COMMON.H"
#include "EGA.H"
#include "GAME.H"
#include "IO.H"
#include "MOUSE.H"
#include "QUIZ.H"
#include "SCRATCH.H"
#include "UTIL.H"
#include "VIDEO.H"

static uchar input_active;

static void draw_hotspots() {
  VIEW *t = current_view->targets;
  while (t) {
    if (t->id != VIEW_TRANSITION && t->id != VIEW_ITEM) {
      int frame = video_get_draw_frame(&s_hotspot);
      hotspot_draw(t->hs_x, t->hs_y, t->sprite_y, frame);
    }
    t = t->next;
  }
}

static void draw_sprites() {
  SPRITE *s = current_view->sprites;
  while (s) {
    animate(s);
    video_draw_sprite(s);
    s = s->next;
  }
}

static void draw_inputs() {
  VIEW *v = current_view;
  if (v->input) {
    if (v->input->clue)
      draw_input(v->input, 0, input_active);
    else
      video_text(0, v->clue, v->input->x, v->input->y, 15, 0);
  }
}

static int mouse_over_hotspot(VIEW *t) {
  return mouse_x >= t->hs_x && mouse_x <= t->hs_x + t->hs_w &&
         mouse_y >= t->hs_y && mouse_y <= t->hs_y + t->hs_h;
}

static int mouse_over_clue(CLUE *c) {
  return mouse_x >= c->x && mouse_x <= c->x + c->w && mouse_y >= c->y &&
         mouse_y <= c->y + c->h;
}

static int clicked_hotspot() {
  VIEW *t = current_view->targets;
  while (t) {
    if (mouse_over_hotspot(t)) {
      mouse_hide();
      if (t->id == VIEW_TRANSITION) {
        game_switch_scene(t->targets);
        input_active = 0;
        draw_sprites();
        draw_hotspots();
      } else {
        // switch to this view
        audio_sfx(SFX_LOOK);
        if (input_active) {
          input_active = 0;
          if (current_view->input)
            draw_input(current_view->input, 0, 0);
        }
        game_push_view(t);
        show_view(t);
        draw_inputs();
        draw_sprites();
        draw_hotspots();
      }
      mouse_show();
      return 1;
    }
    t = t->next;
  }
  return 0;
}

static int clicked_clue() {
  CLUE *c = current_view->popup.clues;
  if (!input_active) {
    while (c) {
      if (mouse_over_clue(c)) {
        quiz_mark(c);
        return 1;
      }
      c = c->next;
    }
  }
  return 0;
}

static int clicked_input() {
  uchar new_active = 0;
  if (current_view->input && current_view->input->clue &&
      mouse_over_input(current_view->input) && !completed) {
    new_active = 1;
  }
  if (input_active != new_active) {
    input_active = new_active;
    mouse_hide();
    draw_input(current_view->input, 0, input_active);
    mouse_show();
    audio_sfx(SFX_DROP);
    return 1;
  }
  return 0;
}

static int clicked_word() {
  WORD *w;

  if (!input_active || !current_view->input || completed)
    return 0;

  w = word_clicked();
  if (w) {
    current_view->input->word = w;
    input_active = 0;
    audio_sfx(SFX_DROP);
    mouse_hide();
    draw_input(current_view->input, 0, input_active);
    mouse_show();
    return 1;
  }

  return 0;
}

static int clicked_quiz() {
  if (mouse_left_clicked() && mouse_over_quiz()) {
    if (in_quiz)
      quiz_exit();
    else
      quiz_enter();
    return 1;
  }
  return 0;
}

static void update_cursor() {
  int c = M_CURSOR_DEFAULT;
  VIEW *t = current_view->targets;
  CLUE *cl = current_view->popup.clues;
  if (mouse_over_quiz()) {
    c = M_CURSOR_NOTE;
  } else {
    if (!input_active) {
      while (cl && c == M_CURSOR_DEFAULT) {
        if (mouse_over_clue(cl)) {
          c = M_CURSOR_NOTE;
        }
        cl = cl->next;
      }
    }
    while (t && c == M_CURSOR_DEFAULT) {
      if (mouse_over_hotspot(t)) {
        if (t->id == VIEW_TRANSITION)
          c = M_CURSOR_MOVE;
        else
          c = M_CURSOR_HOTSPOT;
      }
      t = t->next;
    }
    if (current_view->input && current_view->input->clue && !completed) {
      if (mouse_over_input(current_view->input))
        c = M_CURSOR_NOTE;
      if (input_active && mouse_over_any_word())
        c = M_CURSOR_NOTE;
    }
  }
  mouse_cursor(c);
}

static void check_keyboard() {
  keyboard_poll();
#ifndef WEB  
  if (keyboard_c == KEY_ESC) {
    quitting = 1;
  }
#endif  
#ifdef DEBUG
  if (keyboard_c == 's') {
    ega_set_active_page(SPRITE_PAGE);
  }
  if (keyboard_c == 'z') {
    ega_set_active_page(0);
  }
#endif
}

void game_frame() {
  update_cursor();
  video_vsync();

  if (second_passed) {
    mouse_hide();
    animate(&s_hotspot);

    draw_sprites();
    draw_hotspots();
#ifdef DEBUG
    video_draw_debug();
#endif
    mouse_show();
  }

  if (mouse_left_clicked()) {
    if (!clicked_clue()) {
      if (!clicked_hotspot()) {
        if (!clicked_word()) {
          if (!clicked_input()) {
            if (game_pop_view()) {
              mouse_hide();
              video_close_popup();
              mouse_show();
            }
          }
        }
      }
    }
  }
}

int main() {
  check_mem();
  io_init();
  video_init();
  audio_init();
  assets_init_1();
  video_fade_out();
  audio_play("MUSIC.IMF1", "MUSIC.IMF2");
  // title
  {
    const uint x = 320 - 13 * 8;
    uchar skip = 0;
    uchar c = 0;
    video_clear(0, 0);
    video_load_image("TITLE.RMG", 0, 0, 0, IMG_SIZE_FULL);
    video_text(0, "mausimus and joker present", x, 16, 15, 0);
    video_text(0, "a deduction mini game for", x, 160, 15, 0);
    video_text(0, " DOSember Game Jam 2025", x, 174, 15, 0);
    mouse_init();
    mouse_cursor(M_CURSOR_HOTSPOT);
    mouse_show();
    video_fade_in();
    while (!quitting && !skip) {
      mouse_poll();
      check_keyboard();
      if (mouse_left_clicked())
        skip = 1;
    }
  }
  video_fade_out();
  mouse_hide();
  mouse_cursor(M_CURSOR_DEFAULT);
  assets_init_2();
  game_init();
  quiz_init();
  load_scene(current_view);

  draw_hotspots();
  mouse_show();
  video_fade_in();

  while (!quitting) {
    video_tick_frame();
    mouse_poll();
    check_keyboard();

    if (!clicked_quiz()) {
      if (in_quiz) {
        quiz_frame();
      } else {
        game_frame();
      }
    }
  }

  audio_end();
  video_end();
  dump_mem();
  io_end();
  return 0;
}
