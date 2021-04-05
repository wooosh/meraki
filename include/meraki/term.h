#pragma once

#include "output.h"
#include "input.h"

#include <stdbool.h>

struct MerakiTerm;

// TODO: accept configuration for unicode, terminal fd, colors, etc
struct MerakiTerm *meraki_term_create();
void meraki_term_destroy(struct MerakiTerm **m);

// all functions return true on success

bool meraki_term_raw(struct MerakiTerm *m);
// you will almost always want to put meraki_term_restore in an atexit handler
// to gracefully crash/exit, otherwise when your program exits the terminal
// will be misconfigured
bool meraki_term_restore(struct MerakiTerm *m);

// writes the terminal size into the parameters given
void meraki_term_size(struct MerakiTerm *m, size_t *width, size_t *height);

// TODO: assert this:
// the terminal must be in raw mode for the input and output modules to work 
struct MerakiOutput *meraki_term_output(struct MerakiTerm *m);
struct MerakiInput *meraki_term_input(struct MerakiTerm *m);
