/* Quid Pro Quo
 * A deduction mini-game by mausimus and joker for DOSember Game Jam 2025
 * https://github.com/mausimus/dos2025
 * MIT License
 */

#if !defined(AUDIO_H)
#define AUDIO_H

#include "COMMON.H"

// #define NO_MUSIC
// #define NO_AUDIO

#define SFX_LOOK 0
#define SFX_MOVE 1
#define SFX_DROP 2
#define SFX_CLUE 3
#define SFX_WON 4

extern int game_time;

void audio_init();
void audio_end();
void audio_stop();
void timer_start(uint hdsec);
void timer_delay(uint hdsec);
void timer_wait();
void audio_sfx(int no);
void audio_play(const char *filename1, const char* filename2);

#endif