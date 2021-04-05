#include <meraki/term.h>
#include <meraki/output.h>
#include <meraki/input.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <stdio.h>

struct MerakiTerm {
  // int resizefd;
  bool raw_mode;

  size_t height;
  size_t width;

  struct termios old_termios;

  // for these to work we need to be in raw mode
  struct MerakiOutput *output;
  struct MerakiInput *input;  
};

struct MerakiTerm *meraki_term_create() {
  struct MerakiTerm *m = malloc(sizeof(struct MerakiTerm));

  if (m != NULL) {
    memset(m, 0, sizeof(struct MerakiTerm));
  }

  return m;
}

void meraki_term_destroy(struct MerakiTerm **m) {
  if ((*m)->output) meraki_output_destroy(&(*m)->output);
  if ((*m)->input) meraki_input_destroy(&(*m)->input);

  free(*m);

  *m = NULL;
}

bool meraki_term_raw(struct MerakiTerm *m) {
  if (m->raw_mode) return true;

  if (tcgetattr(STDIN_FILENO, &m->old_termios) == -1) {
    return false;
  }

  // TODO|CLEANUP: write descriptions for each flag
  // https://viewsourcecode.org/snaptoken/kilo/02.enteringRawMode.html
  struct termios new_termios = m->old_termios;
  new_termios.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  new_termios.c_oflag &= ~(OPOST);
  new_termios.c_cflag |= (CS8);
  new_termios.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  new_termios.c_cc[VMIN] = 0;
  new_termios.c_cc[VTIME] = 1;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &new_termios) == -1) {
    return false;
  }

  printf("\x1b[?1049h" // save current screen contents
         "\x1b[2J");   // clear screen
  fflush(stdout);

  
  // TODO|FEATURE: handle resizing
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  
  m->height = w.ws_row;
  m->width = w.ws_col;

  m->raw_mode = true;
  
  return true;
}


bool meraki_term_restore(struct MerakiTerm *m) {
  if (m->raw_mode) {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &m->old_termios) == -1) {
      return false;
    }
    m->raw_mode = false;
  }

  return true;
}

struct MerakiOutput *meraki_term_output(struct MerakiTerm *m) {
  if (m->raw_mode && m->output == NULL) {
    m->output = meraki_output_create(m->height);
  }

  return m->output;
}

struct MerakiInput *meraki_term_input(struct MerakiTerm *m) {
  if (m->raw_mode && m->input == NULL) {
    m->input = meraki_input_create(m->height);
  }

  return m->input;
}
