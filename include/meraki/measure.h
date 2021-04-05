#pragma once

#include <sys/types.h>
#include <wchar.h>

// TODO: explain locale stuff
// TODO: explain the difference between byte and screen pos

/*
  a custom measuring function can be provided to the measuring functions,
  which is useful for implementing your own width rules (like expanding tabs
  to certain number of spaces, or converting control characters to their
  hexadecimal equivalent. the function should return the number of terminal
  cells to advance, or -1 to indicate that the default measuring function
  should be used
*/
typedef int (*meraki_measure_func)(wchar_t);

struct MerakiRange {
  size_t start;
  size_t end;
};

// TODO: define what happens when a byte is a diacritical

// these functions expect either a custom measuring function or NULL, and a
// properly encoded string based on the current LC_CTYPE
struct MerakiRange meraki_byte_to_screen_idx(meraki_measure_func m, size_t idx,
                                             size_t len, char *str);
struct MerakiRange meraki_screen_to_byte_idx(meraki_measure_func m, size_t idx,
                                             size_t len, char *str);
