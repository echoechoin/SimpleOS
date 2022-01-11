#include "screen.h"
#include "ports.h"
#include "charbitmap.h"
#include "util.h"
#include "stdio.h"

static void set_palette(int color, unsigned char r, unsigned char g, unsigned char b);
void draw_pixel(unsigned char color, int x, int y);

void init_palette(void) {
    int i;
    static unsigned char table_rgb[16 * 3] = {
        0x00, 0x00, 0x00, // 黑色
        0xff, 0x00, 0x00, // 亮红色
        0x00, 0xff, 0x00, // 亮绿色
        0xff, 0xff, 0x00, // 亮黄色
        0x00, 0x00, 0xff, // 亮蓝色
        0xff, 0x00, 0xff, // 亮紫色
        0x00, 0xff, 0xff, // 浅亮蓝色
        0xff, 0xff, 0xff, // 白色
        0xc6, 0xc6, 0xc6, // 亮灰色
        0x84, 0x00, 0x00, // 暗红色
        0x00, 0x84, 0x00, // 暗绿色
        0x84, 0x84, 0x00, // 暗黄色
        0x00, 0x00, 0x84, // 暗蓝色
        0x84, 0x00, 0x84, // 暗紫色
        0x00, 0x84, 0x84, // 浅暗蓝色
        0x84, 0x84, 0x84  // 暗灰色
    };
    
    for (i = 0; i < 16; i++) {
        _io_cli();
        int eflags = _io_load_eflags();

        set_palette(
            i, 
            table_rgb[i * 3 + 0], 
            table_rgb[i * 3 + 1], 
            table_rgb[i * 3 + 2]
        );
        
        _io_sti();
        _io_restore_eflags(eflags);
    }
}

void init_mouse(char *mouse, char background) {
    static char cursor[16][16] = {
        "**************..",
        "*OOOOOOOOOOO*...",
        "*OOOOOOOOOO*....",
        "*OOOOOOOOO*.....",
        "*OOOOOOOO*......",
        "*OOOOOOO*.......",
        "*OOOOOOO*.......",
        "*OOOOOOOO*......",
        "*OOOO**OOO*.....",
        "*OOO*..*OOO*....",
        "*OO*....*OOO*...",
        "*O*......*OOO*..",
        "**........*OOO*.",
        "*..........*OOO*",
        "............*OO*",
        ".............***"
    };

    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            if (cursor[y][x] == '*') {
                mouse[y * 16 + x] = COL8_BLACK;
            }

            if (cursor[y][x] == 'O') {
                mouse[y * 16 + x] = COL8_WHITE;
            }

            if (cursor[y][x] == '.') {
                mouse[y * 16 + x] = background;
            }
        }
    }
} 

void draw_mouse(int x, int y, char*mouse) {
    int index = 0;
    int i, j;
    for (i = 0; i < 16; i++) {
        for (j = 0; j < 16; j++) {
            draw_pixel(mouse[index++], i + x, j + y); 
        }
    }
}

static void set_palette(int color, unsigned char r, unsigned char g, unsigned char b) {
    port_byte_out(0x03c8, color);
    port_byte_out(0x03c9, r / 4);
    port_byte_out(0x03c9, g / 4);
    port_byte_out(0x03c9, b / 4);
}

void draw_pixel(unsigned char color, int x, int y) {
    unsigned char *vram = (unsigned char *)VGA_ADDRESS;
    vram[y * SCREEN_WIDTH + x] = color;
}

void draw_char(unsigned char color, int x, int y, char c) {
    int i, j;
    for (i = 0; i < 16; i++) {
        for (j = 0; j < 8; j++) {
            if (charbitmap[c * 16 + i] & (0x80 >> j)) {
                draw_pixel(color, x + j, y + i);
            }
        }
    }
}

void draw_string(unsigned char color, int x, int y, char *str) {
    for (; *str != 0; str++) {
        draw_char(color, x, y, *str);
        x += 8;
    }
}

void draw_rectangle(unsigned char color, int x0, int y0, int x1, int y1) {
    int x, y;
    for (y = y0; y <= y1; y++) {
        for (x = x0; x <= x1; x++) {
            draw_pixel(color, x, y);
        }
    }
}

void init_screen(int x, int y) {
    draw_rectangle(COL8_DARK_GREY,       0,      0,      x - 1,  y - 29);
    draw_rectangle(COL8_LIGHT_GREY,      0,      0,      x - 1,  y - 28);
    draw_rectangle(COL8_WHITE,           0,      0,      x - 1,  y - 27);
    draw_rectangle(COL8_LIGHT_GREY,      0,      y - 26, x - 1,  y - 1 );

    draw_rectangle(COL8_DARK_GREY,       3,      y - 24, 59,     y - 24);
    draw_rectangle(COL8_LIGHT_GREY,      2,      y - 24, 2,      y - 4 );
    draw_rectangle(COL8_WHITE,           3,      y - 4,  59,     y - 4 );
    draw_rectangle(COL8_LIGHT_GREY,      59,     y - 23, 59,     y - 5 );
    draw_rectangle(COL8_WHITE,           2,      y - 4,  59,     y - 3 );
    draw_rectangle(COL8_WHITE,           60,     y - 24, 60,     y - 3 );

    draw_rectangle(COL8_DARK_GREY,       x - 47, y - 24, x - 4,  y - 24);
    draw_rectangle(COL8_LIGHT_GREY,      x - 47, y - 23, x - 47, y - 4 );
    draw_rectangle(COL8_WHITE,           x - 47, y - 3,  x - 4,  y - 3 );
    draw_rectangle(COL8_WHITE,           x - 3,  y - 24, x - 3,  y - 3 );
}