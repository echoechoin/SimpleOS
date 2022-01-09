#include "../drivers/screen.h"
#include "util.h"

void main() {

    init_palette();

    init_screen(320, 200);
    draw_char(COL8_RED, 0,  0, '1');
    draw_char(COL8_RED, 8,  0, '2');
    draw_char(COL8_RED, 16, 0, '3');
    
    for (;;);
}
