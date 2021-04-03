#pragma once

#include "output.h"
#include "input.h"

#include <stdbool.h>

struct MerakiTerm;
extern const struct MerakiTerm MerakiTermInit;

// all functions return true on success

bool meraki_term_raw(struct MerakiTerm *m);
// you will almost always want to put meraki_term_restore in an atexit handler
// to gracefully crash/exit, otherwise when your program exits the terminal
// will be misconfigured
bool meraki_term_restore(struct MerakiTerm *m);

// TODO: assert this:
// the terminal must be in raw mode for the input and output modules to work 
struct MerakiOutput *meraki_term_output(struct MerakiTerm *m);
struct MerakiInput *meraki_term_input(struct MerakiTerm *m);
