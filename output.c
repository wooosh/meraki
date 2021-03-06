#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <inttypes.h>

#include <meraki/output.h>

// TODO: document internals

/*
  Hashing
  
  this is an implementation of the FNV-1a 32 bit hash
  based off of http://www.isthe.com/chongo/src/fnv/hash_32a.c
*/

typedef uint32_t Hash;

#define FNV_PRIME ((Hash) 0x01000193)
#define FNV_START ((Hash) 0x811c9dc5)

static void hash_feed(Hash* h, size_t len, void* buffer) {
  uint8_t *cursor = buffer;
  uint8_t *end = cursor + len;

  while (cursor < end) {
    *h ^= (Hash) *cursor++;
    *h *= FNV_PRIME;
  }
}

/*
  Memory Management
*/

struct MerakiOutput {
  // stores output buffer
  char *out;
  size_t out_len;
  size_t out_cap;

  struct MerakiStyle current_style;

  Hash *line_hashes;
  // TODO: automatically update on resize
  size_t line_hashes_len;
  size_t line_hashes_cap;
};

// resizes the memory to fit len if neccesary, uses power of 2 resizing
// returns the new memory address or null
static void *ensure_len(void *mem, size_t *cap, size_t len, size_t elem_size) {
  if (len > *cap) {
    if (*cap == 0) {
      *cap = 1;
    }
    
    while (*cap < len)
      *cap *= 2;

    mem = realloc(mem, *cap * elem_size);
  }

  return mem;
}

static void write_char(struct MerakiOutput *m, char c) {
  m->out_len++;
  m->out = ensure_len(m->out, &m->out_cap, m->out_len, 1);
  m->out[m->out_len - 1] = c;
}

static void write_str(struct MerakiOutput *m, char* str) {
  size_t insert_point = m->out_len;
  size_t len = strlen(str);

  m->out_len += len;
  m->out = ensure_len(m->out, &m->out_cap, m->out_len, 1);

  memcpy(m->out + insert_point, str, len);
}

// BUG: newly allocated lines are uninitialized memory, and need to be set to
// zero
bool meraki_output_resize(struct MerakiOutput *m, size_t height) {
  m->line_hashes_len = height;
  m->line_hashes = ensure_len(m->line_hashes, &m->line_hashes_cap,
                              m->line_hashes_len, sizeof(Hash));

  return m->line_hashes != NULL;
}

// TODO: take allocator
struct MerakiOutput *meraki_output_create(size_t height) {
  struct MerakiOutput *m = malloc(sizeof(struct MerakiOutput));
  if (m == NULL)
    return NULL;

  memset(m, 0, sizeof(struct MerakiOutput));
  
  m->current_style = (struct MerakiStyle){{Meraki8Color, -1}, {Meraki8Color, -1}, MerakiNone};

  if (!meraki_output_resize(m, height)) {
    free(m);
    return NULL;
  }

  return m;
}

void meraki_output_destroy(struct MerakiOutput **m) {
  free((*m)->out);
  free((*m)->line_hashes);
  free(*m);

  *m = NULL;
}


/*
  Line Formatting
*/
struct MerakiAttrCode {
  enum MerakiAttr attr;
  char code; 
};

#define NUM_ATTRS 6
struct MerakiAttrCode attr_codes[] = {
  {MerakiBright, '1'},
  {MerakiDim, '2'},
  {MerakiUnderscore, '4'},
  {MerakiBlink, '5'},
  {MerakiReverse, '7'},
  {MerakiHidden, '8'},
};

static bool meraki_color_equal(struct MerakiColor a, struct MerakiColor b) {
  if (a.type == b.type) {
    // TODO: MerakiIndexColor
    if (a.type == Meraki8Color) {
      return a.color16 == b.color16;
    } else if (a.type == MerakiTruecolor) {
      return (a.color_true.r == b.color_true.r) &&
             (a.color_true.g == b.color_true.g) &&
             (a.color_true.b == b.color_true.b);
    }
  }
  return false;
}

// color escape is 'XXX;'
#define MAX_COLOR_ESC_LEN 3 + 1
// returns number of characters written
static int meraki_set_color(size_t len, char *buffer, struct MerakiColor prev, struct MerakiColor next) {
  // TODO: truecolor
  if (meraki_color_equal(prev, next)) return 0;
  
  if (next.type == Meraki8Color) {
    // reset needs no escape code
    if (next.color16 == -1) return 0;
    // TODO: handle negative return value
    return snprintf(buffer, len, "%" PRId16 ";", next.color16);
  } else {
    assert(0);
  }
}


/* allocate space for:
    - fg and bg color escapes
    - each attr + its semicolon
    - null terminator
*/
#define MAX_STYLE_LEN (MAX_COLOR_ESC_LEN*2 + NUM_ATTRS*2 + 1)
// TODO: colors
// returns a null terminated string with the escape codes neccesary to 
// transition between styles
static void meraki_output_set_style(struct MerakiOutput *m, 
                                    struct MerakiStyle next) {
  struct MerakiStyle prev = m->current_style;
  char buffer[MAX_STYLE_LEN];
  size_t cur = 0;
  if (prev.attr != next.attr) {
    // keep enough space for the buffer for escape code each attr + a separator 
    // and null terminator
  
    for (int i=0; i<NUM_ATTRS; i++) {
      if (next.attr & attr_codes[i].attr) {
        buffer[cur] = attr_codes[i].code;
        buffer[cur+1] = ';';
        cur += 2;
      }
    }
  }

  cur += meraki_set_color(MAX_STYLE_LEN - cur, buffer + cur, prev.fg, next.fg);
  cur += meraki_set_color(MAX_STYLE_LEN - cur, buffer + cur, prev.bg, next.bg);

  // if escape codes have been written, move the cursor over the ; so it will
  // be treated like the end of the string
  if (cur != 0) {
    cur--;

    buffer[cur] = 'm';
    buffer[cur+1] = 0;

    write_str(m, "\x1b[0;");
    write_str(m, buffer);
  }
}

// writes a rendered line to the output buffer
static void meraki_output_write_line(struct MerakiOutput *m, size_t len,
                                     char *text, struct MerakiStyle *styling) {
  write_str(m, "\x1b[K");
  for (size_t i = 0; i < len; i++) {
    meraki_output_set_style(m, styling[i]);
    write_char(m, text[i]);
    m->current_style = styling[i];
  }
}


/*
  Public Drawing API
*/

// writes the escape code neccesary to move the cursor to the requested
// position on screen
void meraki_output_cursor_move(struct MerakiOutput *m, size_t x, size_t y) {
  size_t escape_len = snprintf(NULL, 0, "\x1b[%zu;%zuH", y + 1, x + 1);
  // TODO: document properly that the +1 is for null terminator
  m->out = ensure_len(m->out, &m->out_cap, m->out_len + escape_len + 1, 1);

  snprintf(m->out + m->out_len, escape_len + 1, "\x1b[%zu;%zuH", y + 1, x + 1);
  // subtract one from the escape length because it includes a null terminator
  m->out_len += escape_len;
}

void meraki_output_draw(struct MerakiOutput *m, size_t screen_y, size_t len,
                        char *text, struct MerakiStyle *styling) {

  Hash h = FNV_START;
  hash_feed(&h, len, text);
  hash_feed(&h, len * sizeof(struct MerakiStyle), styling);

  if (m->line_hashes[screen_y] != h) {
    meraki_output_cursor_move(m, 0, screen_y);
    meraki_output_write_line(m, len, text, styling);

    m->line_hashes[screen_y] = h;
  }
}

void meraki_output_commit(struct MerakiOutput *m) {
  write(STDOUT_FILENO, m->out, m->out_len);
  m->out_len = 0;
}

void meraki_output_cursor_show(struct MerakiOutput *m) {
  write_str(m, "\x1b[?25h");
}

void meraki_output_cursor_hide(struct MerakiOutput *m) {
  write_str(m, "\x1b[?25l");
}
