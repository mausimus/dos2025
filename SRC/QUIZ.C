/* Quid Pro Quo
 * A deduction mini-game by mausimus and joker for DOSember Game Jam 2025
 * https://github.com/mausimus/dos2025
 * MIT License
 */

#include <CTYPE.H>
#include <STDLIB.H>
#include <STRING.H>

#include "ASSETS.H"
#include "AUDIO.H"
#include "COMMON.H"
#include "GAME.H"
#include "MOUSE.H"
#include "QUIZ.H"
#include "UTIL.H"
#include "VIDEO.H"

#define MAX_INPUT 12
#define MAX_INPUTS 50
#define INPUT_W (MAX_INPUT * 8)
#define INPUT_H 9
#define STATUS_LX (208 - 12 * 8)
#define STATUS_RX (448 - 12 * 8)
#define STATUS_Y 157
#define STATUS_BAD 4
#define STATUS_GOOD 2

uchar in_quiz;
uchar completed;
uchar explain;

static uchar num_inputs;
static INPUTBOX inputs[MAX_INPUTS];

static INPUTBOX *activebox = NULL;
static INPUTBOX *hoverbox = NULL;
static INPUTBOX *redrawbox = NULL;

#define WORDS_TOP (200 - CLUE_HEIGHT * 3)
#define MAX_WORDS 30

static uchar num_words;
static uint word_x = 8;
static uint word_y;
static WORD words[MAX_WORDS];
static int story_stat = 2, portraits_stat = 2;

typedef struct GAP {
  CLUE *clue;
} GAP;

#define STORY_LINES 16
#define NUM_GAPS 22

static const char* gaps[NUM_GAPS] = {
  "",
  BANKER_FIRST,
  ROBBER_FIRST,
  "partners",
  "give",
  "money",
  "revenge",
  BOY_FIRST,
  "passkey",
  "stolen",
  GIRL_FIRST,
  "Heinrich",
  "necklace",
  "date",
  "safe",
  "cut",
  "opened",
  "left",
  "robbery",
  "fingerprint",
  "lovers",
  "alarm"
};

static char* story[STORY_LINES] = {
  "[\x01________] and [\x02________] used to be [\x03________] until [\x01________]",
  "cheated. [\x01________] tried to [\x04________] them [\x05________] to settle.",
  "",
  "[\x02________] spotted an opportunity for [\x06________] and tasked [\x07________]",
  "with arranging a [\x12________] to get [\x09________] [\x05________] back.",
  "",
  "The [\x15________] was disabled using a [\x08________] [\x09________] by [\x0A________]",
  "from [\x0B________], concealed in a [\x0C________] and handed to [\x07________].",
  "",
  "[\x01________] unknowingly [\x11________] the [\x13________] on the [\x0C________]",
  "during the [\x0D________] with [\x0A________] thinking they'd be [\x14________].",
  "",
  "The [\x0E________] was [\x0F________] into by [\x07________] and [\x10________] by",
  "[\x02________]. [\x02________] [\x11________] the [\x05________] in the [\x0E________]",
  "to mark his [\x06________], and [\x11________] the [\x08________] to frame",
  "[\x01________] into the [\x12________]."
};

static const char* solution =
  "  " ROBBER_FIRST " and " BANKER_FIRST " were partners and professional\n"
  "bank robbers. After the heat got too high " BANKER_FIRST "\n"
  "betrayed " ROBBER_FIRST " in exchange for immunity and " ROBBER_FIRST "\n"
  "went to prison.\n\n"
  "  Now " BANKER_FIRST " has been scamming ordinary people\n"
  "using a sham banking house. After coming out of\n"
  "prison and seeing what's happening, " ROBBER_FIRST " decided\n"
  "to teach " BANKER_FIRST " a lesson and use his skills one\n"
  "more time for revenge on his old partner.\n\n\n"
  "                Thanks for playing!";

int mouse_over_input(INPUTBOX *b) {
  return mouse_x >= b->x && mouse_x <= b->x + INPUT_W && mouse_y >= b->y &&
         mouse_y <= b->y + INPUT_H;
}

int mouse_over_word(WORD *w) {
  return mouse_x >= w->x && mouse_x <= w->x + w->w &&
         mouse_y >= w->y + WORDS_TOP && mouse_y <= w->y + w->h + WORDS_TOP;
}

int mouse_over_any_word() {
  int i;
  for (i = 0; i < num_words; i++) {
    if (mouse_over_word(words + i)) {
      return 1;
    }
  }
  return 0;
}

static void update_mouse() {
  int i;

  hoverbox = NULL;

  if (!completed) {
    if (activebox) {
      if (mouse_over_any_word()) {
        mouse_cursor(M_CURSOR_NOTE);
        return;
      }
    }

    for (i = 0; i < num_inputs; i++) {
      if (mouse_over_input(inputs + i)) {
        hoverbox = inputs + i;
      }
    }
  }
  if (hoverbox || mouse_over_quiz()) {
    mouse_cursor(M_CURSOR_NOTE);
  } else {
    mouse_cursor(M_CURSOR_DEFAULT);
  }
}

void draw_quiz_input(INPUTBOX *input) {
  draw_input(input, QUIZ_PAGE, input == activebox);
}

void draw_input(INPUTBOX *input, uchar page, uchar active) {
  char padded_input[MAX_INPUT + 1];
  int i, m;
  const char *text;
  uint textlen = 0;
  uchar bg = INPUT_BG;
  uchar padding = 0;

  if (input->word != NULL) {
    text = input->word->clue->text;

    // special rules
    textlen = strlen(text);
  }
  for (i = 0; i < MAX_INPUT; i++) {
    padded_input[i] = ' ';
  }
  m = (MAX_INPUT - 1 - textlen) / 2;
  for (i = 0; i < textlen; i++) {
    padded_input[i + m] = text[i];
  }
  padded_input[MAX_INPUT] = 0;
  bg = clue_colors[input->clue->type];

  if (input->clue == NULL)
    fatal_error("Input->Clue NULL");

  if (active) {
    video_text(page, "     \x9e     ", input->x, input->y, INPUT_FG, bg);
  } else {
    padded_input[MAX_INPUT - 1] = 0;
    video_text(page, padded_input, input->x, input->y, INPUT_FG, bg);
  }
  video_draw_side(page, input->x - 8, input->y, bg, 0);
  video_draw_side(page, input->x + 88, input->y, bg, 1);
}

void quiz_init() {
  int i, l;
  uint y = 5;
  uint sx = 16;

  lookup_clues(words);

  for (i = 0; i < STORY_LINES; i++) {
    char *c = story[i];
    while (*c) {
      if (*c == '[') {
        INPUTBOX *input = inputs + num_inputs++;
        char text[20];
        CLUE *clue;
        if (num_inputs >= MAX_INPUTS)
          fatal_error("Not enough inputs");

        if (*(c + 10) != ']')
          fatal_error("Bad input");

        strncpy(text, gaps[c[1]], 15);

        for (l = 0; l < 10; l++) c[l] = ' ';

        clue = find_clue(text);
        if (clue != NULL) {
          input->clue = clue;
        } else {
          fatal_errorf("Missing clue %s", text);
        }
        input->word = words;
        input->x = sx + (c - story[i]) * 8;
        input->y = y;
      }
      c++;
    }
    video_text(QUIZ_PAGE, story[i], sx, y, QUIZ_FG, QUIZ_BG);

    if (story[i][0] == 0)
      y += 6;
    else
      y += LINE_HEIGHT + 1;
  }

  for (i = 0; i < num_inputs; i++) {
    draw_quiz_input(inputs + i);
  }

  quiz_check();
}

static void draw_status(const char *text, uint x, uchar bg) {
  video_draw_side(QUIZ_PAGE, x - 8, STATUS_Y, bg, 0);
  video_text(QUIZ_PAGE, text, x, STATUS_Y, 15, bg);
  video_draw_side(QUIZ_PAGE, x + 184, STATUS_Y, bg, 1);
}

static void redraw_quiz() {
  int i;
  uint y = 5;
  uint sx = 16;

  mouse_hide();
  video_vsync();
  video_load_image("GAME\\QUIZ.RMG", QUIZ_PAGE, 0, 0, IMG_SIZE_SHORT);

  for (i = 0; i < STORY_LINES; i++) {
    video_text(QUIZ_PAGE, story[i], sx, y, QUIZ_FG, QUIZ_BG);

    if (story[i][0] == 0)
      y += 6;
    else
      y += LINE_HEIGHT + 1;
  }

  for (i = 0; i < num_inputs; i++) {
    draw_quiz_input(inputs + i);
  }
  draw_status(" Portraits:   Correct! ", STATUS_LX, STATUS_GOOD);
  draw_status(" Story:       Correct! ", STATUS_RX, STATUS_GOOD);
  mouse_show();
}

WORD *word_clicked() {
  int i;
  for (i = 0; i < num_words; i++) {
    if (mouse_over_word(words + i)) {
      return words + i;
    }
  }
  return NULL;
}

static int clicked_word() {
  WORD *w;

  if (!activebox)
    return 0;

  w = word_clicked();
  if (w) {
    activebox->word = w;
    redrawbox = activebox;
    activebox = NULL;
    audio_sfx(SFX_DROP);
    return 1;
  }
  return 0;
}

void quiz_frame() {
  update_mouse();

  redrawbox = NULL;

  if (mouse_left_clicked()) {
    if (explain) {
      redraw_quiz();
      explain = 0;
    } else if (!completed) {
      if (!clicked_word()) {
        INPUTBOX *currentbox = activebox;
        if (hoverbox) {
          activebox = hoverbox;
          redrawbox = activebox;
          audio_sfx(SFX_DROP);
        } else {
          activebox = NULL;
        }

        // redraw one that loses focus (mouse is not on it!)
        if (currentbox && currentbox != hoverbox) {
          mouse_hide();
          draw_quiz_input(currentbox);
          mouse_show();
        }
      } else {
        quiz_check();
      }
    }
  }

#ifdef DEBUG
  if (keyboard_c == 'a')
    quiz_reveal();
  if (keyboard_c == 'c')
    show_all_clues();
#endif

  video_vsync();

  if (redrawbox) {
    mouse_hide();
    draw_quiz_input(redrawbox);
    mouse_show();
  }
}

void quiz_enter() {
  in_quiz = 1;
  mouse_hide();
  quiz_check();
  video_active_page(QUIZ_PAGE);
  mouse_page(QUIZ_PAGE);
  mouse_show();
}

void quiz_exit() {
  in_quiz = 0;
  mouse_hide();
  video_active_page(0);
  mouse_page(0);
  mouse_show();
}

void quiz_mark(CLUE *clue) {
  uchar i, bg;
  WORD *word = words + num_words;
  char ch[2];

  for (i = 0; i < num_words; i++) {
    if (words[i].clue == clue)
      return;
  }

  if (num_words++ == MAX_WORDS)
    fatal_error("Not enough words!");

  word->clue = clue;

  // will it fit?
  if (word_x + strlen(clue->text) * 8 >= (640 - QUIZ_BUTTON_W)) {
    word_x = 8;
    word_y += CLUE_HEIGHT;
  }

  if (num_words > 1)
    audio_sfx(SFX_CLUE);

  bg = clue_colors[clue->type];
  ch[1] = 0;
  mouse_hide();
  video_draw_side(0, word_x - 8, word_y + WORDS_TOP, bg, 0);
  mouse_show();
  for (i = 0; i < strlen(clue->text); i++) {
    ch[0] = clue->text[i];
    video_vsync();
    video_vsync();
    mouse_hide();
    video_text(0, ch, word_x + 8 * i, word_y + WORDS_TOP, WORD_FG, bg);
    mouse_show();
  }
  video_vsync();
  video_vsync();
  mouse_hide();
  video_draw_side(0, word_x + strlen(clue->text) * 8, word_y + WORDS_TOP, bg,
                  1);
  video_text(QUIZ_PAGE, clue->text, word_x, word_y + WORDS_TOP, WORD_FG, bg);
  video_draw_side(QUIZ_PAGE, word_x - 8, word_y + WORDS_TOP, bg, 0);
  video_draw_side(QUIZ_PAGE, word_x + strlen(clue->text) * 8,
                  word_y + WORDS_TOP, bg, 1);
  mouse_show();
  word->x = word_x;
  word->y = word_y;
  word->w = strlen(clue->text) * 8;
  word->h = LINE_HEIGHT;
  word_x += (strlen(clue->text) + 1) * 8;
}

void quiz_explain() {
  uint x = 8, y = 2, w = 624, h = LINE_HEIGHT * 17;

  video_fill(QUIZ_PAGE, x, y, w, h, 15, 15);
  video_draw_edge(QUIZ_PAGE, x, y, w, h, 0, 15, STYLE_PRINT);
  video_text(QUIZ_PAGE, " CONGRATULATIONS! ", 320 - 80, y, 0, 15);
  video_text(QUIZ_PAGE, solution, 120, y + 2 * LINE_HEIGHT, 0, 15);
  video_load_image("ROBBER-S.RMG", QUIZ_PAGE, 16, 56 - 18, IMG_SIZE_SHORT);
  video_load_image("BANKER-S.RMG", QUIZ_PAGE, 640 - 16 - 88, 56 - 18,
                   IMG_SIZE_SHORT);
  video_text(QUIZ_PAGE, ROBBER_FIRST, 40, 56 + 64 - 3 - 18, 0, 15);
  video_text(QUIZ_PAGE, BANKER_FIRST, 640 - 88, 56 + 64 - 3 - 18, 0, 15);
  explain = 1;
}

void quiz_check() {
  uint i;
  int story = 2, portraits = 2;
  INPUTBOX *box = inputs;
  {
    for (i = 0; i < num_inputs && story; i++) {
      if (box->word == NULL || box->word == words) {
        story = 0;
      }
      box++;
    }
    box = inputs;
    if (story == 2) {
      uchar incorrect = 0;
      for (i = 0; i < num_inputs && !incorrect; i++) {
        switch (i) {
          case 0: {
            // accept either order
            CLUE *robber_clue = find_clue(ROBBER_FIRST);
            CLUE *banker_clue = find_clue(BANKER_FIRST);
            INPUTBOX *box2 = inputs + 1;
            if (box->word->clue == robber_clue &&
                box2->word->clue == banker_clue)
              break;
            if (box->word->clue == banker_clue &&
                box2->word->clue == robber_clue)
              break;
            incorrect = 1;
            break;
          }
          case 1:
            break;
          case 23: {
            // accept either word
            CLUE *necklace_clue = find_clue("necklace");
            CLUE *passkey_clue = find_clue("passkey");
            ASSERT(box->clue == necklace_clue);
            if (box->word->clue != necklace_clue &&
                box->word->clue != passkey_clue)
              incorrect = 1;
            break;
          }
          default:
            if (box->word->clue != box->clue) {
              incorrect = 1;
            }
            break;
        }
        box++;
      }
      if (incorrect)
        story = 1;
    }

    if (story_stat != story) {
      story_stat = story;
      mouse_hide();
      switch (story) {
        case 0:
          draw_status(" Story:     Incomplete ", STATUS_RX, STATUS_BAD);
          break;
        case 1:
          draw_status(" Story:      Incorrect ", STATUS_RX, STATUS_BAD);
          break;
        case 2:
          draw_status(" Story:       Correct! ", STATUS_RX, STATUS_GOOD);
          break;
      }
      mouse_show();
    }

    // check portraits
    portraits = inputs_check();
    if (portraits_stat != portraits) {
      portraits_stat = portraits;
      mouse_hide();
      switch (portraits) {
        case 0:
          draw_status(" Portraits: Incomplete ", STATUS_LX, STATUS_BAD);
          break;
        case 1:
          draw_status(" Portraits:  Incorrect ", STATUS_LX, STATUS_BAD);
          break;
        case 2:
          draw_status(" Portraits:   Correct! ", STATUS_LX, STATUS_GOOD);
          break;
      }
      mouse_show();
    }

    if (!completed && story_stat == 2 && portraits_stat == 2) {
      completed = 1;
      mouse_hide();
      activebox = NULL;
      if (redrawbox) {
        draw_quiz_input(redrawbox);
        redrawbox = NULL;
      }
      quiz_explain();
      audio_sfx(SFX_WON);
      mouse_show();
    }
  }
}

void quiz_reveal() {
  uint i, w;
  INPUTBOX *box = inputs;
  show_all_clues();

  mouse_hide();
  for (i = 0; i < num_inputs; i++) {
    for (w = 0; w < num_words - 1; w++) {
      if (words[w].clue == box->clue) {
        box->word = words + w;
        break;
      }
    }
    draw_quiz_input(box);
    box++;
  }
  mouse_show();
  quiz_check();
}