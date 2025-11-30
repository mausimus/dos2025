/* Quid Pro Quo
 * A deduction mini-game by mausimus and joker for DOSember Game Jam 2025
 * https://github.com/mausimus/dos2025
 * MIT License
 */

#if !defined(MOUSE_H)
#define MOUSE_H

#include "COMMON.H"

#define M_CURSOR_DEFAULT 0
#define M_CURSOR_HOTSPOT 1
#define M_CURSOR_MOVE 2
#define M_CURSOR_NOTE 3
#define KEY_ESC 27

extern uint mouse_x, mouse_y;
extern uchar mouse_c;
extern uchar keyboard_c;

void mouse_init();
void mouse_show();
void mouse_hide();
void mouse_poll();
void mouse_page(uchar page);
void mouse_move();
int mouse_left_clicked();
void mouse_cursor(uchar c);
void keyboard_poll();

#endif
