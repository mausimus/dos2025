/* Quid Pro Quo
 * A deduction mini-game by mausimus and joker for DOSember Game Jam 2025
 * https://github.com/mausimus/dos2025
 * MIT License
 */

#if !defined(GAME_H)
#define GAME_H

#include "COMMON.H"
#include "ASSETS.H"

#define GIRL_FIRST "Elsa"
#define GIRL_LAST "Ziegler"
#define BOY_FIRST "Jacek"
#define BOY_LAST "Du\x89""ek"
#define ROBBER_FIRST "Mat\x88j"
#define ROBBER_LAST "Koz\xa0r"
#define BANKER_FIRST "Lorenz"
#define BANKER_INITIAL "L."
#define BANKER_LAST "Gr\x94""del"
#define ASSISTANT_FIRST "Leon"
#define ASSISTANT_LAST "Brotz"

extern VIEW *current_view;
extern VIEW *parent_view;
extern uchar quitting;

void game_push_view(VIEW *view);
int game_pop_view();
void game_init();
void game_switch_scene(VIEW *scene);
int mouse_over_quiz();

#endif