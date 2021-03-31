#pragma once

#include "output.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct MerakiOutput;

// TODO: take allocator
// returns null on failed alloc
struct MerakiOutput *meraki_output_create(size_t height);

void meraki_output_destroy(struct MerakiOutput *m);

// returns true if it could not resize
bool meraki_output_resize(struct MerakiOutput *m, size_t height);

// drawing api
enum MerakiAttr {
  MerakiNone = 1 << 0,
  // bright is also bold
  MerakiBright = 1 << 1,
  MerakiDim = 1 << 2,
  MerakiUnderscore = 1 << 3,
  MerakiBlink = 1 << 4,
  MerakiReverse = 1 << 5,
  MerakiHidden = 1 << 6,
};

enum MerakiColorType { Meraki8Color, Meraki256Color, MerakiTruecolor };

struct MerakiColor {
  // enum MerakiColorType
  uint8_t type;
  union {
    // -1 == reset
    int8_t color16;
    int8_t color256;
    struct {
      uint8_t r;
      uint8_t g;
      uint8_t b;
    } color_true;
  };
};

struct MerakiStyle {
  struct MerakiColor fg;
  struct MerakiColor bg;

  // enum MerakiAttr
  uint8_t attr;
};

bool meraki_output_draw(struct MerakiOutput *m, size_t screen_y, size_t len,
                        char *text, struct MerakiStyle *styling);

void meraki_output_commit(struct MerakiOutput *m);

// TODO: meraki_clear_screen
// TODO: meraki_set_cursor
// TODO: meraki_show_cursor
// TODO: meraki_hide_cursor
// TODO: meraki_cursor_type
// TODO: meraki_output_scroll
