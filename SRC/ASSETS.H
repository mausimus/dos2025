/* Quid Pro Quo
 * A deduction mini-game by mausimus and joker for DOSember Game Jam 2025
 * https://github.com/mausimus/dos2025
 * MIT License
 */

#if !defined(ASSETS_H)
#define ASSETS_H

#define IMG_SIZE(w, h) ((h) * ((w) / 2) + 4)
#define IMG_SIZE_SHORT 54084u
#define IMG_SIZE_FULL 64004u

#define INV_X 120
#define INV_S 64
#define INV_W 64
#define INV_H 30
#define INV_Y 40
#define INV_TEXT(i) (30 + 9 * i)

#define HS_W 16
#define HS_H 8

#include "COMMON.H"

typedef struct SPRITE {
  // location, relative to parent popup
  uint sx;
  uint sy;
  // width/height
  uint w;
  uint h;
  uchar frames;
  const char *filename;
  struct SPRITE *next;
  uchar frame;
  uchar far *data;
  const char *text;
} SPRITE;

typedef enum CLUE_TYPE { Person, Verb, Noun, Wildcard } CLUE_TYPE;

typedef struct CLUE {
  uint x;
  uint y;
  uint w;
  uint h;
  const char *text;
  struct CLUE *next;
  CLUE_TYPE type;
} CLUE;

typedef struct WORD {
  uint x;
  uint y;
  uint w;
  uint h;
  CLUE *clue;
} WORD;

typedef struct INPUTBOX {
  uint x;
  uint y;
  CLUE *clue;  // solution
  WORD *word;  // selection
} INPUTBOX;

typedef enum STYLE {
  STYLE_DEFAULT = 0,
  STYLE_BORDERLESS = 1,
  STYLE_PRINT = 2,
  STYLE_HAND = 4,
  STYLE_JAGGED = 8
} STYLE;

typedef struct POPUP {
  uint x;
  uint y;
  uint w;
  uint h;
  const char *filename;
  const char *message;
  uchar far *data;
  CLUE *clues;
  STYLE style;
} POPUP;

typedef enum VIEWS {
  VIEW_NONE = 0,
  VIEW_TRANSITION,
  VIEW_GENERIC,
  VIEW_ITEM,
  VIEW_INVENTORY
} VIEWS;

// could be main scene or popup
typedef struct VIEW {
  VIEWS id;   // to customize rendering/behavior
  uint hs_x;  // hotspot location (relative to parent)
  uint hs_y;
  uint sprite_y;
  uint num_targets;
  uchar hs_w;
  uchar hs_h;
  const char* clue;
  INPUTBOX *input;
  POPUP popup;           // background
  SPRITE *sprites;       // animations (list)
  struct VIEW *targets;  // places to go from here
  struct VIEW *next;     // for use in targets list
} VIEW;

extern SPRITE s_hotspot;
extern uchar clue_colors[6];
extern CLUE* wildcard;

void show_view(VIEW *p);
void assets_init_1();
void assets_init_2();
void load_scene(VIEW *v);
void animate(SPRITE *s);
SPRITE *create_sprite(const char *filename, int frames);
VIEW *create_image_view(const char *filename);
VIEW *create_text_view(const char *message, STYLE style);
VIEW *create_inventory_view(const char *inventory, const char *portrait,
                            const char *dialog, const char *input);
VIEW *create_fingerprint_view(const char *filename);
SPRITE *add_sprite(VIEW *view, int x, int y, const char *filename,
                   int frames);
SPRITE *add_text_sprite(VIEW *view, int x, int y, const char *text);
VIEW *add_view(VIEW *parent, int x, int y, VIEW *child);
VIEW *add_text(VIEW *parent, int x, int y, const char *message, STYLE style);
VIEW *add_text_item(VIEW *parent, int no, const char *message, STYLE style);
VIEW *add_view_item(VIEW *parent, int no, VIEW *child);
CLUE *create_clue(const char *text);
CLUE *add_clue(VIEW *view, const char *text, const char *word,
               CLUE_TYPE type);
CLUE *add_clue_image(VIEW *view, int x, int y, int w, int h);
VIEW *add_inventory(VIEW *parent, int x, int y, const char *inventory,
                    const char *portrait, const char *dialog,
                    const char *input);
void add_transition(VIEW *from, int x, int y, VIEW *to);
void lookup_clues(WORD* wildcardw);
CLUE *find_clue(const char *text);
int inputs_check();
void show_all_clues();

#endif
