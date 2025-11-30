/* Quid Pro Quo
 * A deduction mini-game by mausimus and joker for DOSember Game Jam 2025
 * https://github.com/mausimus/dos2025
 * MIT License
 */

#include <ALLOC.H>
#include <STDIO.H>
#include <STRING.H>

#include "ASSETS.H"
#include "COMMON.H"
#include "IO.H"
#include "QUIZ.H"
#include "SCRATCH.H"
#include "UTIL.H"
#include "VIDEO.H"

#define MAX_VIEWS 70
#define MAX_SPRITES 40
#define MAX_CLUES 50
#define MAX_INPUTS 10

SPRITE s_hotspot = {0, 32, 16, 8, 3, "GAME\\SPRITES.RMG", NULL, 0, NULL};

VIEW *current_scene;
CLUE *wildcard;
uchar clue_colors[6] = {5, 6, 3, 7, 0, 0};
static uchar view_idx;
static VIEW views[MAX_VIEWS];
static uchar sprite_idx;
static SPRITE sprites[MAX_SPRITES];
static uchar clue_idx;
static CLUE clues[MAX_CLUES];
static uchar input_idx;
static INPUTBOX inputs[MAX_INPUTS];

void load_view(VIEW *view, uchar level);
static void load_popup(POPUP *popup);
void unload_view(VIEW *view);

SPRITE *create_sprite(const char *filename, int frames) {
  SPRITE *s = sprites + sprite_idx++;
  if (sprite_idx >= MAX_SPRITES)
    fatal_error("Insufficient sprites");
  s->filename = filename;
  s->frames = frames;
  if (filename) {
    io_dimensions(filename, &s->w, &s->h);
    ASSERT(s->w % (8 * frames) == 0);
    s->w /= frames;
  }
  return s;
}

CLUE *create_clue(const char *text) {
  uchar i;
  CLUE *c = clues + clue_idx;
  if (clue_idx >= MAX_CLUES)
    fatal_error("Insufficient clues");
  for (i = 0; i < clue_idx; i++) {
    if (strcmp((clues + i)->text, text) == 0)
      fatal_errorf("Duplicate clue '%s'", text);
  }
  c->text = text;
  clue_idx++;
  return c;
}

VIEW *create_image_view(const char *filename) {
  VIEW *v = views + view_idx++;
  if (view_idx >= MAX_VIEWS)
    fatal_error("Insufficient views");
  v->id = VIEW_GENERIC;
  v->popup.filename = filename;
  if (filename) {
    io_dimensions(filename, &v->popup.w, &v->popup.h);
    ASSERT(v->popup.w % 8 == 0);
    v->popup.x = ((640 - v->popup.w) / 2) & 0xFFF8;  // align to 8
    v->popup.y = (170 - v->popup.h) / 2;
  } else {
    // placeholder
    v->popup.x = 240;
    v->popup.y = 50;
    v->popup.w = 160;
    v->popup.h = 100;
  }
  return v;
}

static VIEW *create_transition_view() {
  VIEW *v = views + view_idx++;
  if (view_idx >= MAX_VIEWS)
    fatal_error("Insufficient views");
  v->id = VIEW_TRANSITION;
  return v;
}

void add_transition(VIEW *from, int x, int y, VIEW *to) {
  VIEW *transition = create_transition_view();
  transition->targets = to;
  add_view(from, x, y, transition);
  transition->hs_w = 16;
  transition->hs_h = 14;
}

VIEW *create_text_view(const char *message, STYLE style) {
  uint max_width = 0;
  uint width = 0;
  uint num_lines = 1;
  const char *c = message;

  VIEW *v = views + view_idx++;
  if (view_idx >= MAX_VIEWS)
    fatal_error("Insufficient views");
  v->id = VIEW_GENERIC;
  v->popup.message = message;
  v->popup.style = style;

  // calculate bounding box
  while (*c) {
    if (*c == '\n') {
      if (width > max_width)
        max_width = width;
      width = 0;
      num_lines++;
    } else {
      width++;
    }
    c++;
  }
  if (width > max_width)
    max_width = width;

  if (max_width % 2 == 1)
    max_width++;
  v->popup.w = max_width * 8 + 32;
  v->popup.h = (num_lines + 2) * LINE_HEIGHT;
  v->popup.x = (320 - v->popup.w / 2) & 0xFFF8;
  v->popup.y = 85 - v->popup.h / 2;
  return v;
}

VIEW *create_inventory_view(const char *inventory, const char *portrait,
                            const char *dialog, const char *input) {
  VIEW *v = views + view_idx++;
  if (view_idx >= MAX_VIEWS)
    fatal_error("Insufficient views");
  v->id = VIEW_INVENTORY;
  v->popup.x = (640 - 432) / 2;
  v->popup.y = (170 - 90) / 2;
  v->popup.w = 432;
  v->popup.h = 90;
  v->popup.message = dialog;
  if (portrait) {
    add_sprite(v, 16, 10, portrait, 3);
  }
  if (inventory) {
    if (strchr(inventory, '\\') != NULL)  // path
    {
      add_sprite(v, INV_X, INV_Y, inventory, 1);
    } else {
      add_text_sprite(v, INV_X, INV_TEXT(0), inventory);
    }
  }
  if (input) {
    v->input = inputs + input_idx++;
    v->input->x = 16;
    v->input->y = 73;
    v->clue = input;
    v->input->clue = NULL;
    v->input->word = NULL;
  }
  return v;
}

VIEW *create_fingerprint_view(const char *filename) {
  VIEW *v = create_image_view(filename);
  v->popup.style = STYLE_BORDERLESS;
  return v;
}

SPRITE *add_sprite(VIEW *view, int x, int y, const char *filename,
                   int frames) {
  SPRITE *sprite = create_sprite(filename, frames);
  sprite->next = view->sprites;
  sprite->sx = x;
  sprite->sy = y;
  view->sprites = sprite;
  return sprite;
}

SPRITE *add_text_sprite(VIEW *view, int x, int y, const char *text) {
  SPRITE *sprite = create_sprite(NULL, 0);
  sprite->next = view->sprites;
  sprite->sx = x;
  sprite->sy = y;
  sprite->text = text;
  view->sprites = sprite;
  return sprite;
}

VIEW *add_view(VIEW *parent, int x, int y, VIEW *child) {
  child->hs_x = x & 0xFFF8;
  child->hs_y = y;
  child->hs_w = HS_W;
  child->hs_h = HS_H;
  child->next = parent->targets;
  parent->targets = child;
  return child;
}

VIEW *add_text(VIEW *parent, int x, int y, const char *message, STYLE style) {
  VIEW *v = create_text_view(message, style);
  add_view(parent, x, y, v);
  return v;
}

VIEW *add_text_item(VIEW *parent, int no, const char *message, STYLE style) {
  VIEW *v = create_text_view(message, style);
  add_view(parent, INV_X + no * INV_S, INV_Y, v);
  v->id = VIEW_ITEM;
  v->hs_w = INV_W;
  v->hs_h = INV_H;
  return v;
}

VIEW *add_view_item(VIEW *parent, int no, VIEW *child) {
  add_view(parent, INV_X + no * INV_S, INV_Y, child);
  child->id = VIEW_ITEM;
  child->hs_w = INV_W;
  child->hs_h = INV_H;
  return child;
}

CLUE *add_clue_image(VIEW *view, int x, int y, int w, int h) {
  CLUE *clue = create_clue(NULL);
  const char *c = view->popup.message;
  clue->x = x;
  clue->y = y;
  clue->w = w;
  clue->h = h;
  clue->next = view->popup.clues;
  view->popup.clues = clue;
  return clue;
}

CLUE *add_clue(VIEW *view, const char *text, const char *word,
               CLUE_TYPE type) {
  CLUE *clue = create_clue(word == NULL ? text : word);
  const char *c = view->popup.message;
  clue->next = view->popup.clues;
  clue->type = type;
  clue->w = strlen(text) * 8;
  clue->h = 10;
  view->popup.clues = clue;

  if (c) {
    // calculate clue position
    const char *clue_pos = stristr(c, text);
    if (clue_pos) {
      while (c != clue_pos) {
        if (*c == '\n') {
          clue->x = 0;
          clue->y += LINE_HEIGHT;
        } else {
          clue->x += 8;
        }
        c++;
      }
    } else {
      fatal_errorf("Clue %s not found in message!", text);
    }
    clue->x += 16;
    clue->y += 8;
  }
  return clue;
}

CLUE *find_clue(const char *text) {
  uchar i;
  uint tl = strlen(text);
  if (text[tl - 1] == 's')
    tl--;
  // exact match
  for (i = 0; i < clue_idx; i++) {
    CLUE *c = clues + i;
    if (strnicmp(c->text, text, tl + 1) == 0) {
      return c;
    }
  }
  // partial match
  for (i = 0; i < clue_idx; i++) {
    CLUE *c = clues + i;
    if (strnicmp(c->text, text, tl) == 0) {
      return c;
    }
  }
  return NULL;
}

VIEW *add_inventory(VIEW *parent, int x, int y, const char *inventory,
                    const char *portrait, const char *dialog,
                    const char *input) {
  VIEW *inv = create_inventory_view(inventory, portrait, dialog, input);
  add_view(parent, x, y, inv);
  return inv;
}

void assets_init_1() {
  video_clear(SPRITE_PAGE, 0);
  video_load_image("GAME\\SPRITES.RMG", SPRITE_PAGE, 0, 0, IMG_SIZE_SHORT);
  video_load_image("GAME\\QUIZ.RMG", QUIZ_PAGE, 0, 0, IMG_SIZE_SHORT);
  video_load_image("GAME\\BOTTOM.RMG", QUIZ_PAGE, 0, 169, IMG_SIZE_SHORT);
  video_text(QUIZ_PAGE, " TO \nGAME", 640 - QUIZ_BUTTON_W + 16,
             200 - QUIZ_BUTTON_H + 6, 7, 0);
}

void assets_init_2() {
  video_load_image("GAME\\BOTTOM.RMG", 0, 0, 169, IMG_SIZE_SHORT);
  video_text(0, " TO \nQUIZ", 640 - QUIZ_BUTTON_W + 16,
             200 - QUIZ_BUTTON_H + 6, 7, 0);

  wildcard = create_clue("\xad\xae\xaf");
  wildcard->type = Wildcard;
  quiz_mark(wildcard);
}

static void generate_sprites(VIEW *p) {
  VIEW *t;
  t = p->targets;
  p->num_targets = 0;
  while (t) {
    // generate frames
    if (t->id != VIEW_ITEM) {
      t->sprite_y = hotspot_push(t->hs_x, t->hs_y);
      p->num_targets++;
    }
    t = t->next;
  }
}

void load_scene(VIEW *v) {
  if (current_scene) {
    hotspot_clear();
    unload_view(current_scene);
    current_scene = NULL;
  }
  video_load_image(v->popup.filename, 0, 0, 0, IMG_SIZE_SHORT);
  load_view(v, 0);
  generate_sprites(v);
  current_scene = v;
}

void show_view(VIEW *p) {
  video_show_view(p);
  generate_sprites(p);
}

void unload_view(VIEW *v) {
  VIEW *target = v->targets;
  SPRITE *sprite = v->sprites;
  if (v->popup.data) {
    free_mem(v->popup.data, IMG_SIZE(v->popup.w, v->popup.h));
    v->popup.data = NULL;
  }
  while (sprite) {
    free_mem(sprite->data, IMG_SIZE(sprite->w * sprite->frames, sprite->h));
    sprite->data = NULL;
    sprite = sprite->next;
  }
  while (target) {
    if (target->id != VIEW_TRANSITION)
      unload_view(target);
    target = target->next;
  }
}

void load_view(VIEW *v, uchar level) {
  VIEW *target = v->targets;
  SPRITE *sprite = v->sprites;
  if (level > 0 && v->popup.filename) {
    load_popup(&v->popup);
  }
  while (sprite) {
    video_load_sprite(sprite);
    sprite = sprite->next;
  }
  while (target) {
    if (target->id != VIEW_TRANSITION)
      load_view(target, level + 1);
    target = target->next;
  }
}

void load_popup(POPUP *popup) {
  uint size = IMG_SIZE(popup->w, popup->h);
  popup->data = (uchar far *)get_mem(size);
  if (popup->data == NULL) {
    char msg[100];
    test_mem();
    sprintf(msg, "Requested %u bytes", size);
    fatal_error(msg);
  }
  io_load(popup->filename, size, popup->data);
}

void animate(SPRITE *s) {
  // there are 2f - 2 drawable frames (loop)
  if (s->frames > 1) {
    s->frame++;
    if (s->frame == s->frames * 2 - 2)
      s->frame = 0;
  }
}

void lookup_clues(WORD *wildcardw) {
  uint i;
  for (i = 0; i < view_idx; i++) {
    VIEW *v = views + i;
    if (v->clue) {
      v->input->clue = find_clue(v->clue);
      v->input->word = wildcardw;
    }
  }
}

int inputs_check() {
  uint i;
  for (i = 0; i < input_idx; i++) {
    INPUTBOX *input = inputs + i;
    if (input->clue && input->word && input->word->clue == wildcard)
      return 0;  // incomplete
  }
  for (i = 0; i < input_idx; i++) {
    INPUTBOX *input = inputs + i;
    if (input->clue && input->word && input->word->clue != input->clue)
      return 1;  // incorrect
  }
  return 2;
}

void show_all_clues() {
  uchar c;
  for (c = 0; c < clue_idx; c++) {
    quiz_mark(clues + c);
  }
}
