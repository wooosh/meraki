#include "meraki.h"
#include "output.h"
#include "input.h"

#include <stdbool.h>
#include <sys/ioctl.h>

const struct MerakiTerm MerakiTermInit = {0}; 

struct MerakiTerm {
  // int resizefd;
  bool raw_mode;

  size_t height;
  size_t width;

  struct termios old_termios;

  // for these to work we need to be in raw mode
  struct MerakiOutput *out;
  struct MerakiInput *in;  
};

bool meraki_term_raw(struct *MerakiTerm m) {
  if (m->raw_mode) return;

  if (tcgetattr(STDIN_FILENO, &m->old_termios) != -1) {
    return false;
  }

  // TODO|CLEANUP: write descriptions for each flag
  // https://viewsourcecode.org/snaptoken/kilo/02.enteringRawMode.html
  struct termios new_termios = old_termios;
  new_termios.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  new_termios.c_oflag &= ~(OPOST);
  new_termios.c_cflag |= (CS8);
  new_termios.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  new_termios.c_cc[VMIN] = 0;
  new_termios.c_cc[VTIME] = 1;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &new_termios) != -1) {
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


void meraki_term_restore(struct *MerakiTerm m) {
  if (m->raw_mode) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, m->old_termios);
  }
}

struct MerakiOutput *meraki_term_output(struct *MerakiTerm m) {
  if (m->raw_mode) {
    if (m->output == NULL) {
      m->output = meraki_output_create(m->height);
    }

    return m->output;
  }

  return NULL;
}
