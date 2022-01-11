#include "screen.h"
#include "util.h"
#include "stdio.h"

void main() {
    int x, y;
    int i = 0;
    init_palette();
    init_screen(320, 200);
    char s[20] = {'1','2',0};
    // draw_char(COL8_RED, 0, 0, 'H');
    int ret = sprintf(s, "vga_address: %d", VGA_ADDRESS);
    draw_string(COL8_RED, 0, 0, s);

    
    for (;;) {
        __asm("hlt");
    }
}
