#include <stdio.h>
#include <string.h>
#include <locale.h>

#include <measure.h>
#include <output.h>

void main() {
  setlocale(LC_CTYPE, "");

  char *str = "aaaaa\u0300aa\u030Aabaa😳";
  char under[100];
  memset(under, ' ', 100);
  under[100] = '\0';

  for (int i=0; i<strlen(str); i++) {
    if (str[i] != 'a') {
      struct MerakiRange r = meraki_byte_to_screen_idx(NULL, i, strlen(str), str);
      memset(under + r.start, '^', r.end - r.start); 
      i += r.end - r.start;
    }
  }
  printf("%s\n", str);
  printf("%s\n", under);
}
