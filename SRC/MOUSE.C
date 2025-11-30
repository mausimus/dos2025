/* Quid Pro Quo
 * A deduction mini-game by mausimus and joker for DOSember Game Jam 2025
 * https://github.com/mausimus/dos2025
 * MIT License
 */

#include "MOUSE.H"
#include "UTIL.H"

uint mouse_x = 0, mouse_y = 0;
uchar mouse_c = 0;
uchar keyboard_c = 0;
static uchar mouse_b = 0, prev_mouse_b = 0;
static uint s_hotspots[8] = {2, 1, 5, 3, 8, 5, 4, 2};

#include "CURSORS.INC"

void mouse_init() {
  int mouse_present;

  asm {
    mov ax, 0
    int 0x33
    mov mouse_present,ax
  }

  if (!mouse_present) {
    fatal_error("Mouse not found!");
  }

  mouse_c = 255;
  mouse_cursor(0);
}

void mouse_show() {
  asm {
    mov ax, 1
    int 0x33
  }
}

void mouse_hide() {
  asm {
    mov ax, 2
    int 0x33
  }
}

void mouse_poll() {
  prev_mouse_b = mouse_b;
  asm {
    mov ax, 3
    int 0x33
    mov mouse_b, bl
    mov mouse_x, cx
    mov mouse_y, dx
  }
}

void mouse_page(uchar page) {
  asm {
    mov ax, 0x1d
    mov bx, 0
    mov bl, page
    int 0x33
  }
}

int mouse_left_clicked() {
  return (mouse_b & 1) && (!(prev_mouse_b & 1));
}

void mouse_cursor(uchar c) {
  uint *shape, hx, hy;
  if (mouse_c == c)
    return;
  mouse_c = c;
  shape = s_cursors + 32 * c;
  hx = s_hotspots[2 * c];
  hy = s_hotspots[2 * c + 1];
  asm {
    push es
    mov bx, hx
    mov cx, hy
    mov dx, ds
    mov es, dx
    mov dx, [shape]
    mov ax, 9
    int 0x33
    pop es
  }
}

void keyboard_poll() {
  // get key, skip next if extended
  keyboard_c = 0;
  asm {
    mov ah, 6
    mov dl, 0xff
    int 0x21
    jz done
    mov keyboard_c, al
    cmp al, 0
    jnz done
    mov ah, 6
    mov dl, 0xff
    int 0x21
  }
  done:
}