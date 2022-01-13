#include "screen.h"
#include "stdio.h"
#include "interrupt.h"
#include "dsctbl.h"
#include "ports.h"
#include "fifo.h"

void main() {
    char buf_mouse[256];
    char s[25] = {0};
    init_gdt();
    init_idt();
    init_pic();
    init_palette();
    init_screen(SCREEN_WIDTH, SCREEN_HEIGHT);
    init_mouse_cursor(buf_mouse, COL8_WHITE);
    init_keyboard();
    init_mouse();

    fifo_bytes_init(&fifo_key, KEY_FIFO_BUF_SIZE, key_buf);
    fifo_bytes_init(&fifo_mouse, MOUSE_FIFO_BUF_SIZE, mouse_buf);

    _io_sti();
    port_byte_out(PIC0_IMR, 0xf9); // 开放PIC1以及键盘中断
    port_byte_out(PIC1_IMR, 0xef); // 开放鼠标中断
    draw_string(COL8_BLUE, 0, 0, "Hello, OS!");

    for (;;) {
        _io_cli();
        if (fifo_bytes_count(&fifo_key) + fifo_bytes_count(&fifo_mouse) == 0) {
            _io_sti();
            __asm__ __volatile__("hlt");
            continue;
        }
        unsigned char data;
        char s[25] = {0};
        int i = 0;
        if (fifo_bytes_count(&fifo_key) != 0) {
            i = fifo_bytes_get(&fifo_key, &data);
            if (i != 0) {
                draw_string(COL8_WHITE, 0, 0, "fifo_key overflow!");
            }
            sprintf(s, "key: %02x", data);
            draw_rectangle(COL8_WHITE, 0, 16, 80, 32);
            draw_string(COL8_BLACK, 0, 16, s);
        } else if (fifo_bytes_count(&fifo_mouse) != 0) {
            i = fifo_bytes_get(&fifo_mouse, &data);
            if (i != 0) {
                draw_string(COL8_WHITE, 0, 0, "fifo_mouse overflow!");
            }
            sprintf(s, "mouse: %02x", data);
            draw_rectangle(COL8_WHITE, 0, 32, 80, 64);
            draw_string(COL8_BLACK, 0, 32, s);
        }
        
    }

    
    for (;;) {
        __asm__ __volatile__("hlt");
    }
}
