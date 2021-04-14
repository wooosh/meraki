#pragma once

#include <limits.h>
#include <stdbool.h>

struct MerakiKey {
  enum MerakiKeyBase {
    // TODO: figure out how to handle backspace properly
    MerakiEscape = 0x1b,

    // All values before Backspace use their ASCII value
    MerakiBackspace = 127,
    MerakiDelete = UCHAR_MAX,

    // Navigation Keys
    MerakiLeftArrow,
    MerakiRightArrow,
    MerakiUpArrow,
    MerakiDownArrow,

    MerakiHome,
    MerakiEnd,

    MerakiPageUp,
    MerakiPageDown,

    // Used to denote an error state
    MerakiKeyNone
  } base;
  bool control;
  bool alt;
  bool shift;
};

struct MerakiInput;

// returns NULL if it is unable to create an input
struct MerakiInput *meraki_input_create();
void meraki_input_destroy(struct MerakiInput **m);

struct MerakiKey meraki_read_key(struct MerakiInput *m);
