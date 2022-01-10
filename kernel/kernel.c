#include "../drivers/screen.h"
#include "util.h"

void main() {
    int x, y;
    int i = 0;
    init_palette();
    init_screen(320, 200);
    
    for (y = 0; y < 200 && i != 256; y += 20) {
        for (x = 0; x < 320 && i != 256; x += 8) {
            draw_char(COL8_GREEN, x, y, i);
            i++;
        }
    }
    
    for (;;) {
        __asm("hlt");
    }
}
