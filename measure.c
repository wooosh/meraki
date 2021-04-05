#include <meraki/measure.h>

#include <assert.h>
#include <stdlib.h>
#include <sys/types.h>
#include <wchar.h>

static size_t meraki_wcwidth(meraki_measure_func m, wchar_t wc) {
  if (m != NULL) {
    int width = m(wc);
    if (width != -1)
      return width;
  }

  int width = wcwidth(wc);
  if (width != -1)
    return width;
  return 0;
}

struct MerakiRange meraki_byte_to_screen_idx(meraki_measure_func m, size_t idx,
                                             size_t len, char *str) {
  // TODO: refactor
  size_t byte_cursor = 0;
  size_t screen_pos = 0;
  wchar_t wc;
  mbstate_t mbstate = {0};
  struct MerakiRange r = {0, 0};

  do {
    int step = mbrtowc(&wc, str + byte_cursor, len - byte_cursor, &mbstate);
    // assert that we are not at the end of the string and have valid utf8
    assert(step > 0);

    size_t width = meraki_wcwidth(m, wc);
    // TODO: argument that triggers diacriticals to be expanded into the char
    // they are modifying
    if (width != 0) {
      r.start = screen_pos;
      r.end = screen_pos + width;
    }
    screen_pos += meraki_wcwidth(m, wc);
    byte_cursor += step;
  } while (byte_cursor <= idx);

  return r;
}

struct MerakiRange meraki_screen_to_byte_idx(meraki_measure_func m, size_t idx,
                                             size_t len, char *str) {
  assert(0);
}
