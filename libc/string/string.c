#include "string.h"

int strlen(const char *s) {
  int i = 0;
  while (*s++) {
    i++;
  }
  return i;
}