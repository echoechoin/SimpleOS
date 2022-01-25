#include "window.h"

void init_window(unsigned char *buf, int xsize, int ysize, char *title) {
  static char closebtn[14][16] = {
      "OOOOOOOOOOOOOOO@",
      "OQQQQQQQQQQQQQ$@",
      "OQQQQQQQQQQQQQ$@",
      "OQQQ@@QQQQ@@QQ$@",
      "OQQQQ@@QQ@@QQQ$@",
      "OQQQQQ@@@@QQQQ$@",
      "OQQQQQQ@@QQQQQ$@",
      "OQQQQQ@@@@QQQQ$@",
      "OQQQQ@@QQ@@QQQ$@",
      "OQQQ@@QQQQ@@QQ$@",
      "OQQQQQQQQQQQQQ$@",
      "OQQQQQQQQQQQQQ$@",
      "O$$$$$$$$$$$$$$@",
      "@@@@@@@@@@@@@@@@"};

  draw_rectangle(buf, xsize, COL8_LIGHT_GREY, 0, 0, xsize - 1, 0);
  draw_rectangle(buf, xsize, COL8_WHITE, 1, 1, xsize - 2, 1);
  draw_rectangle(buf, xsize, COL8_LIGHT_GREY, 0, 0, 0, ysize - 1);
  draw_rectangle(buf, xsize, COL8_WHITE, 1, 1, 1, ysize - 2);
  draw_rectangle(buf, xsize, COL8_DARK_YELLOW, xsize - 2, 1, xsize - 2, ysize - 2);
  draw_rectangle(buf, xsize, COL8_BLACK, xsize - 1, 0, xsize - 1, ysize - 1);
  draw_rectangle(buf, xsize, COL8_LIGHT_GREY, 2, 2, xsize - 3, ysize - 3);
  draw_rectangle(buf, xsize, COL8_DAEK_BLUE, 3, 3, xsize - 4, 20);
  draw_rectangle(buf, xsize, COL8_DARK_GREY, 1, ysize - 2, xsize - 2, ysize - 2);
  draw_rectangle(buf, xsize, COL8_BLACK, 0, ysize - 1, xsize - 1, ysize - 1);
  draw_string(buf, xsize, 24, 4, COL8_WHITE, title);

  for (int y = 0; y < 14; y++) {
    for (int x = 0; x < 16; x++) {
      char c = closebtn[y][x];
      switch (c) {
      case '@':
        c = COL8_BLACK;
        break;
      case '$':
        c = COL8_DARK_GREY;
        break;
      case 'Q':
        c = COL8_LIGHT_GREY;
        break;
      default:
        c = COL8_WHITE;
        break;
      }
      buf[(5 + y) * xsize + (xsize - 21 + x)] = c;
    }
  }
}