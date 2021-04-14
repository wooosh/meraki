#include <assert.h>
#include <ctype.h>
#include <poll.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <meraki/input.h>

// TODO: expose errors in a cleaner way instead of using assert
// TODO|CLEANUP: describe how the terminal input system works at the top

#define MAX_ESC_LEN 6
struct MerakiInput {
  // stores the available characters, null terminated
  char seq[MAX_ESC_LEN + 1];
  // how many characters are remaining in the buffer
  size_t available;
};

struct MerakiInput *meraki_input_create() {
  struct MerakiInput *m = malloc(sizeof(struct MerakiInput));
  
  if (m != NULL) {
    memset(m, 0, sizeof(struct MerakiInput));
  }

  return m;
}

void meraki_input_destroy(struct MerakiInput **m) {
  free(*m);
  *m = NULL;
}

// TODO: make this non blocking
// refills the seq buffer. sets available to 0 in the event of an error
static void input_queue_refill(struct MerakiInput *inqueue) {
  // if there is data available, only check for new immediately available data
  int timeout = -1;
  if (inqueue->available > 0) {
    timeout = 0;
  }

  struct pollfd pfd = {STDIN_FILENO, POLLIN};
  int ready = poll(&pfd, 1, timeout);
  // TODO: expose errno somehow
  // maybe make an error field in the MerakiInput, with a union for errno and
  // pollerr/pollhup? we can then return MerakiKeyError and have a union of 
  // modifiers OR error info 
  if (ready == -1) {
    inqueue->available = 0;
    return;
  }

  if (ready > 0) {
    if (pfd.revents & POLLIN) {
      // refill buffer
      ssize_t result = read(STDIN_FILENO, inqueue->seq + inqueue->available,
                           MAX_ESC_LEN - inqueue->available);
      // TODO: expose errno somehow
      if (result == -1) {
        inqueue->available = 0;
        return; 
      }

      inqueue->available += result;
      // insert null terminator
      inqueue->seq[inqueue->available] = '\0';
    } else { // POLLERR | POLLHUP
      // TODO: expose POLLERR/POLLHUP somehow
      inqueue->available = 0;
    }
  }
}

// TODO|CLEANUP: document
static void input_queue_consume(struct MerakiInput *inqueue, size_t used) {
  memmove(inqueue->seq, inqueue->seq + used, inqueue->available - used);
  inqueue->available -= used;
}

struct EscapeRule {
  enum MerakiKeyBase key;
  // null terminated
  char pattern[MAX_ESC_LEN + 1];

  // determines if the key supports modifiers
  bool mod;
};

struct EscapeRule escape_rules[] = {
    // TODO: remove "\x1b[" prefix
    {MerakiDelete, "\x1b[3~"},
    {MerakiPageDown, "\x1b[5~"},
    {MerakiPageUp, "\x1b[6~"},

    {MerakiUpArrow, "\x1b[A", true},
    {MerakiDownArrow, "\x1b[B", true},
    {MerakiRightArrow, "\x1b[C", true},
    {MerakiLeftArrow, "\x1b[D", true},
    {MerakiHome, "\x1b[H", true},
    {MerakiEnd, "\x1b[F", true},

    {MerakiKeyNone}};

static bool starts_with(char *str, char *prefix) {
  return strncmp(str, prefix, strlen(prefix)) == 0;
}

// TODO|BUG: does this split up unicode characters????
struct MerakiKey meraki_read_key(struct MerakiInput *inqueue) {
  input_queue_refill(inqueue);
  if (inqueue->available < 1) return (struct MerakiKey){MerakiKeyNone};

  if (starts_with(inqueue->seq, "\x1b[")) {
    for (int i = 0; escape_rules[i].key != MerakiKeyNone; i++) {
      struct EscapeRule rule = escape_rules[i];

      if (starts_with(inqueue->seq, rule.pattern)) {
        input_queue_consume(inqueue, strlen(rule.pattern));
        return (struct MerakiKey){rule.key};
      } else if (rule.mod && inqueue->available >= 6) {
        /* Keys with modifiers held down use the following format:
         *    "\x1b[1;NX";
         *            ^ ^- key type      (index 5)
         *            |- modifier number (index 4)
         */

        // Make sure it matches the format
        if (!starts_with(inqueue->seq, "\x1b[1;"))
          continue;
        if (rule.pattern[2] != inqueue->seq[5])
          continue;

        struct MerakiKey k = {rule.key};
        // convert ascii char into int and subtract one to turn it into a
        // bitfield. i'm not sure why the bitfield has one added to it in the
        // first place though :/
        int mod = inqueue->seq[4] - 48 - 1;

        k.shift = mod & (1 << 0);
        k.alt = mod & (1 << 1);
        k.control = mod & (1 << 2);

        input_queue_consume(inqueue, 6);

        return k;
      }
    }
  }

  struct MerakiKey k = {inqueue->seq[0], .shift = isupper(inqueue->seq[0]),
                  .control = iscntrl(inqueue->seq[0])};

  // the ASCII code for ctrl-letter is letter - 0x60, so we convert control
  // characters base key to their alphabet version, with the exception of \r
  // because it represents enter, and \x1b since it represents escape
  if (k.control && k.base != '\r' && k.base != '\x1b') {
    k.base += 0x60;
  }
  
  // convert control-h backspace
  if (k.control && k.base == 'h') {
    k.control = false;
    k.base = MerakiBackspace;
  }

  input_queue_consume(inqueue, 1);
  return k;
}
