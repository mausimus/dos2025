/* Quid Pro Quo
 * A deduction mini-game by mausimus and joker for DOSember Game Jam 2025
 * https://github.com/mausimus/dos2025
 * MIT License
 */

#if !defined(QUIZ_H)
#define QUIZ_H

#include "ASSETS.H"
#include "COMMON.H"

extern uchar in_quiz;
extern uchar completed;

void draw_input(INPUTBOX *input, uchar page, uchar active);
int mouse_over_any_word();
int mouse_over_input(INPUTBOX *b);
WORD* word_clicked();

void quiz_init();
void quiz_enter();
void quiz_frame();
void quiz_exit();
void quiz_check();
void quiz_reveal();
void quiz_mark(CLUE *clue);

#endif
