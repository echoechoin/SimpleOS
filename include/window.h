#ifndef _WINDOW_H_
#define _WINDOW_H_
#include "screen.h"

void init_window(unsigned char *buf, int xsize, int ysize, char *title);
void init_textbox(struct SHEET *sheet, int x0, int y0, int slen);

#endif