#ifndef _SCREEN_H_
#define _SCREEN_H_

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200
#define VGA_ADDRESS 0xa0000

#define COL8_BLACK           0
#define COL8_RED             1
#define COL8_GREEN           2
#define COL8_YELLOW          3
#define COL8_BLUE            4
#define COL8_PURPLE          5
#define COL8_LIGHT_BLUE      6
#define COL8_WHITE           7
#define COL8_LIGHT_GREY      8
#define COL8_DARK_RED        9
#define COL8_DARK_GREEN      10
#define COL8_DARK_YELLOW     11
#define COL8_DAEK_BLUE       12
#define COL8_DARK_PURPLE     13
#define COL8_DARK_LIGHT_BLUE 14
#define COL8_DARK_GREY       15

void init_palette(void);
void init_screen(unsigned char *vram, int x, int y);
void init_mouse_cursor(char *mouse, char background);

void draw_pixel(unsigned char *vram, int xsize,unsigned char color, int x, int y);
void draw_mouse(unsigned char *vram, int xsize,int x, int y, char*mouse);
void draw_rectangle(unsigned char *vram, int xsize,unsigned char color, int x0, int y0, int x1, int y1);
void draw_char(unsigned char *vram, int xsize,unsigned char color, int x, int y, char c);
void draw_string(unsigned char *vram, int xsize, unsigned char color, int x, int y, char *str);

#endif
