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
    unsigned memtotal;
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
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
    md.mx = SCREEN_WIDTH / 2;
    md.my = SCREEN_HEIGHT / 2;
    draw_mouse(md.mx, md.my, buf_mouse);

    memtotal = memtest(0x00400000, 0xbfffffff);
    memman_init(memman);
    memman_free(memman, 0x00001000, 0x0009e000);
    memman_free(memman, 0x00400000, memtotal - 0x00400000);
    sprintf(s, "memory %dMB", memtotal / (1024 * 1024));
    draw_string(COL8_BLACK, 0, 16, s);
    sprintf(s, "memory free %dKB", memman_total(memman) / 1024);
    draw_string(COL8_BLACK, 0, 32, s);

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
            draw_rectangle(COL8_WHITE, 0, 48, 80, 64);
            draw_string(COL8_BLACK, 0, 48, s);
        } else if (fifo_bytes_count(&fifo_mouse) != 0) {
            i = fifo_bytes_get(&fifo_mouse, &data);
            _io_sti();
            if (mouse_decode(&md, data) != 0) {
                sprintf(s, "mouse: %d %d %d", md.x, md.y, md.btn);
                draw_rectangle(COL8_WHITE, 0, 64, 200, 80);
                draw_string(COL8_BLACK, 0, 64, s);
                int x0 = md.mx - md.x;
                int y0 = md.my - md.y;
                int x1 = x0 + 16;
                int y1 = y0 + 16;
                if (x0 < 0) {
                    x0 = 0;
                }
                if (y0 < 0) {
                    y0 = 0;
                }
                if (x1 > SCREEN_WIDTH) {
                    x1 = SCREEN_WIDTH;
                }
                if (y1 > SCREEN_HEIGHT) {
                    y1 = SCREEN_HEIGHT;
                }
                draw_rectangle(COL8_WHITE,x0, y0, x1, y1);
                draw_mouse(md.mx, md.my, buf_mouse);
            }
        }
        
    }
}
