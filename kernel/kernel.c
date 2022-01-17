#include "screen.h"
#include "stdio.h"
#include "interrupt.h"
#include "dsctbl.h"
#include "ports.h"
#include "fifo.h"
#include "memory.h"

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
    struct mouse_desc md = {0};
    draw_mouse(md.mx, md.my, buf_mouse);
    int i = memset(0x40000000, 0xbfffffff) /(1024*1024);
    sprintf(s, "mem: %d MB", i);
    draw_string(COL8_BLACK, 0, 48, s);

    for (;;) {
        _io_cli();
        if (fifo_bytes_count(&fifo_key) + fifo_bytes_count(&fifo_mouse) == 0) {
            __asm__ __volatile__("sti");
            __asm__ __volatile__("hlt");
            continue;
        }
        unsigned char data;
        char s[25] = {0};
        int i = 0;
        if (fifo_bytes_count(&fifo_key) != 0) {
            i = fifo_bytes_get(&fifo_key, &data);
            sprintf(s, "key: %02x", data);
            draw_rectangle(COL8_WHITE, 0, 16, 80, 32);
            draw_string(COL8_BLACK, 0, 16, s);
        } else if (fifo_bytes_count(&fifo_mouse) != 0) {
            i = fifo_bytes_get(&fifo_mouse, &data);
            _io_sti();
            if (mouse_decode(&md, data) != 0) {
                sprintf(s, "mouse: %d %d %d", md.x, md.y, md.btn);
                draw_rectangle(COL8_WHITE, 0, 16, 200, 32);
                draw_string(COL8_BLACK, 0, 16, s);
                draw_rectangle(COL8_WHITE, md.mx - md.x, md.my -md.y, md.mx - md.x+ 16, md.my -md.y + 16);
                draw_mouse(md.mx, md.my, buf_mouse);
            }
        }
        
    }
}
