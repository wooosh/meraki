#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <assert.h>
#include <unistd.h>

// debug
#include <stdio.h>

#include <hash.h>
#include <output.h>

struct MerakiOutput {
  // stores output buffer
  char *out;
  size_t out_len;
  size_t out_cap;

  Hash *line_hashes;
  // TODO: automatically update on resize
  size_t line_hashes_len;
  size_t line_hashes_cap;
};

/*
  Memory Management
*/

// resizes the memory to fit len if neccesary, uses power of 2 resizing
// returns the new memory address or null
static void *ensure_len(void *mem, size_t *cap, size_t len, size_t elem_size) {
  if (len > *cap) {
    size_t old_cap = *cap;
    if (*cap == 0) {
      *cap = 1;
    }
    *cap *= 2;

    mem = realloc(mem, *cap * elem_size);
    //memset(mem + old_cap, 0, *cap - old_cap);
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

  if (!meraki_output_resize(m, height)) {
    free(m);
    return NULL;
  }

  return m;
}

void meraki_output_destroy(struct MerakiOutput *m) {
  free(m->out);
  free(m->line_hashes);

  memset(m, 0, sizeof(struct MerakiOutput));
}

// writes the escape code neccesary to move the cursor to the requested
// position on screen
static void meraki_output_move_to(struct MerakiOutput *m, size_t x, size_t y) {
  size_t escape_len = snprintf(NULL, 0, "\x1b[%zu;%zuH", y + 1, x + 1);
  m->out = ensure_len(m->out, &m->out_cap, m->out_len + escape_len, 1);

  snprintf(m->out + m->out_len, escape_len, "\x1b[%zu;%zuH", x + 1, y + 1);
  m->out_len += escape_len;
}

/*
  Line Formatting
*/
struct MerakiAttrCode {
  enum MerakiAttr attr;
  char code; 
};

#define NUM_ATTRS 7
struct MerakiAttrCode attr_codes[] = {
  {MerakiNone, '0'},
  {MerakiBright, '1'},
  {MerakiDim, '2'},
  {MerakiUnderscore, '4'},
  {MerakiBlink, '5'},
  {MerakiReverse, '7'},
  {MerakiHidden, '8'},
};

// TODO: colors
// returns a null terminated string with the escape codes neccesary to 
// transition between styles
static void meraki_output_set_style(struct MerakiOutput *m, 
                                    struct MerakiStyle prev,
                                    struct MerakiStyle next) {
  if (prev.attr == next.attr) return;

  // keep enough space for the buffer for escape code each attr + a separator 
  // and null terminator
  static char buffer[NUM_ATTRS*2 + 1];
  size_t cur = 0;

  for (int i=0; i<NUM_ATTRS; i++) {
    enum MerakiAttr attr = attr_codes[i].attr;
    if (!(prev.attr & attr) && next.attr & attr) {
      buffer[cur] = attr_codes[i].code;
      buffer[cur+1] = ';';
      cur += 2;
    }
  }

  if (cur != 0) {
    buffer[cur-1] = 'm';
    buffer[cur] = 0;

    write_str(m, "\x1b[0;");
    write_str(m, buffer);
  }
}

// writes a rendered line to the output buffer
static void meraki_output_write_line(struct MerakiOutput *m, size_t len,
                                     char *text, struct MerakiStyle *styling) {
  struct MerakiStyle prev = {
      {Meraki8Color, -1}, {Meraki8Color, -1}, MerakiNone};
  for (size_t i = 0; i < len; i++) {
    meraki_output_set_style(m, prev, styling[i]);
    write_char(m, text[i]);
    prev = styling[i];
  }
}


/*
  Public Drawing API
*/
bool meraki_output_draw(struct MerakiOutput *m, size_t screen_y, size_t len,
                        char *text, struct MerakiStyle *styling) {

  Hash h;
  hash_init(&h);
  hash_feed(&h, len, text);
  hash_feed(&h, len * sizeof(struct MerakiStyle), styling);

  if (m->line_hashes[screen_y] != hash_result(&h)) {
    // meraki_output_move_to(m, 0, screen_y);
    meraki_output_write_line(m, len, text, styling);

    m->line_hashes[screen_y] = hash_result(&h);
  }
}

void meraki_output_commit(struct MerakiOutput *m) {
  write(STDOUT_FILENO, m->out, m->out_len);
  m->out_len = 0;
}
