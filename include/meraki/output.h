#pragma once

#include "output.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// TODO: document its unicode awareness (or lack thereof)

struct MerakiOutput;

// TODO: take allocator
// returns null on failed alloc
struct MerakiOutput *meraki_output_create(size_t height);

void meraki_output_destroy(struct MerakiOutput **m);

// returns true if it could not resize
bool meraki_output_resize(struct MerakiOutput *m, size_t height);

// TODO: remove MerakiNone
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

// TODO: change Meraki8Color and Meraki256Color to MerakiIndexColor
// TODO: MerakiTruecolor -> MerakiRGB
enum MerakiColorType { Meraki8Color, MerakiTruecolor };

struct MerakiColor {
  // TODO: try to get rid of the type
  // enum MerakiColorType
  uint8_t type;
  union {
    // -1 == reset
    int16_t color16;
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

// TODO: handle failed allocations
void meraki_output_draw(struct MerakiOutput *m, size_t screen_y, size_t len,
                        char *text, struct MerakiStyle *styling);

// display currently buffered changes
void meraki_output_commit(struct MerakiOutput *m);

void meraki_output_cursor_show(struct MerakiOutput *m);
void meraki_output_cursor_hide(struct MerakiOutput *m);
void meraki_output_cursor_move(struct MerakiOutput *m, size_t x, size_t y);

// TODO: meraki_clear_screen
// TODO: meraki_cursor_type
// TODO: meraki_output_scroll
