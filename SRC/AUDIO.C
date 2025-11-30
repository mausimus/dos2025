/* Quid Pro Quo
 * A deduction mini-game by mausimus and joker for DOSember Game Jam 2025
 * https://github.com/mausimus/dos2025
 * MIT License
 */

#include <DOS.H>
#include <STDIO.H>

#include "ADLIB.H"
#include "AUDIO.H"
#include "COMMON.H"
#include "IO.H"
#include "UTIL.H"

#define TIMER_BASE 1193182l
#define FREQ_DIV 560
#define TIME_CNT (FREQ_DIV / 2)
#define SFX_VOICE 0

typedef struct ADLIB_CMD {
  uchar reg;
  uchar data;
  uint delay;
} ADLIB_CMD;

typedef struct ADLIB_SONG {
  uchar far *data;
  uint len;
  uint ofs;
} ADLIB_SONG;

typedef struct ADLIB_SFX {
  int i;
  int freq;
  int duration;
} ADLIB_SFX;

static ADLIB_SONG song1;
static ADLIB_SONG song2;
static ADLIB_SONG *song;
static ADLIB_CMD far *cmd;

int game_time;
static unsigned long org_step, org_cnt;
static long music_cnt, next_cnt;
static uint wait_cnt, sound_cnt, time_cnt = TIME_CNT;

static ADLIB_INST instruments[5];
// clang-format off
static ADLIB_SFX sfx[5] = {
  {0, 220, 280},
  {1, 440, 250},
  {2, 220, 280},
  {3, 880, 280},
  {4, 523, 25}
};
// clang-format on

typedef struct {
  int stopFreq;
  long freqMult;
} FREQ_MAP;

// clang-format off
static FREQ_MAP freq_map[8] = {
  {48, 2109},
  {97, 1054},
  {194, 527},
  {388, 263},
  {776, 131},
  {1552, 65},
  {3104, 32},
  {6208, 16}
};
// clang-format on

void interrupt(far *org_handler)();

static void music_tick();

void interrupt isr_handler()  // new ISR handler
{
  // sfx
  if (sound_cnt) {
    sound_cnt--;
    if (!sound_cnt)
      adlib_stop(SFX_VOICE);
  }

  // music
  music_tick();

  // wait timer
  if (wait_cnt)
    wait_cnt--;

  // game timer
  time_cnt--;
  if (!time_cnt) {
    time_cnt = TIME_CNT;
    game_time++;
  }

  // old interrupt
  org_cnt += org_step;
  if (org_cnt > 0xffffl) {
    org_cnt &= 0xffffl;
    org_handler();
  } else {
    outportb(0x20, 0x20);
  }
}

static void next_cmd() {
  //char msg[40];
  //sprintf(msg, "%u %u %lu %lu   ", song->ofs, song->len, music_cnt, next_cnt);
  //video_text(0, msg, 0, 0, 15, 0);

  if (song->ofs >= song->len) {
    if (song == &song1) {
      // second part
      song = &song2;
      //video_text(0, "PART 2", 0, 0, 15, 0);
    } else {
      // loop
      //video_text(0, "PART 1", 0, 0, 15, 0);
      song = &song1;
      music_cnt = 0;
      next_cnt = 0;
    }
    song->ofs = 0;
    cmd = (ADLIB_CMD far *)song->data;
  } else {
    cmd++;
    song->ofs += sizeof(ADLIB_CMD);
  }
}

static void music_tick() {
  if (cmd == NULL)
    return;
  music_cnt++;
  while (music_cnt >= next_cnt) {
    adlib_out(cmd->reg, cmd->data);
    next_cnt += cmd->delay;
    next_cmd();
  }
}

void timer_delay(uint hdsec) {
  timer_start(hdsec);
  timer_wait();
}

void timer_start(uint hdsec) {
  wait_cnt = (FREQ_DIV / 20u) * hdsec;
}

void timer_wait() {
  while (wait_cnt);
}

void audio_init() {
#ifndef NO_AUDIO
  adlib_load("SOUND\\PIZZI.SBI", instruments + SFX_MOVE);
  adlib_load("SOUND\\BREATH.SBI", instruments + SFX_LOOK);
  adlib_load("SOUND\\WOODBLOC.SBI", instruments + SFX_DROP);
  adlib_load("SOUND\\PIZZ.SBI", instruments + SFX_CLUE);
  adlib_load("SOUND\\VIBES.SBI", instruments + SFX_WON);

  adlib_reset();
  adlib_volume(SFX_VOICE, 0);
#endif

  org_step = TIMER_BASE / FREQ_DIV;
  outportb(0x43, 0x36);
  outportb(0x40, org_step & 0xff);
  outportb(0x40, org_step >> 8);
  org_handler = getvect(8);
  setvect(8, isr_handler);
}

void audio_end() {
  if (org_handler) {
    outportb(0x43, 0x36);
    outportb(0x40, 0xff);
    outportb(0x40, 0xff);
    setvect(8, org_handler);
    audio_stop();
  }
}

void audio_beep(int freq) {
#ifndef NO_AUDIO
  uint div = (uint)(TIMER_BASE / freq);
  uchar tmp;

  outportb(0x43, 0xB6);
  outportb(0x42, (char)div);
  outportb(0x42, (char)(div >> 8));

  tmp = inportb(0x61);
  outportb(0x61, tmp | 3);

  timer_delay(2);

  // stop
  tmp = inportb(0x61) & 0xfc;
  outportb(0x61, tmp);
#endif
}

static void play_freq(uchar voice, int freq) {
  int i, v;
  for (i = 0; i < 8; i++) {
    if (freq < freq_map[i].stopFreq) {
      v = (int)(freq * freq_map[i].freqMult / 100);
      adlib_play(voice, v, i);
      return;
    }
  }
}

void audio_sfx(int no) {
#ifndef NO_AUDIO
  ADLIB_SFX *s = sfx + no;
  adlib_stop(SFX_VOICE);
  adlib_set(SFX_VOICE, instruments + s->i);
  play_freq(0, s->freq);
  sound_cnt = s->duration;
  if (no == SFX_WON) {
    static int freqs[7] = {293 * 2, 329 * 2, 349 * 2, 392 * 2,
                           440 * 2, 493 * 2, 523 * 2};
    int i = 0;
    for (i = 0; i < 7; i++) {
      while (sound_cnt);
      play_freq(0, freqs[i]);
      if (i == 6)
        sound_cnt = s->duration * 2;
      else
        sound_cnt = s->duration;
    }
  }
#endif
}

void audio_play(const char *filename1, const char *filename2) {
#ifndef NO_MUSIC
  uint len;

  len = io_len(filename1);
  song1.data = get_mem(len);
  io_load(filename1, len, song1.data);
  song1.ofs = 0;
  song1.len = len;

  len = io_len(filename2);
  song2.data = get_mem(len);
  io_load(filename2, len, song2.data);
  song2.ofs = 0;
  song2.len = len;

  song = &song1;
  music_cnt = 0;
  next_cnt = 0;
  cmd = (ADLIB_CMD far *)song->data;
#endif
}

void audio_stop() {
#ifndef NO_AUDIO
  adlib_stop(SFX_VOICE);
  adlib_reset();
#endif
}