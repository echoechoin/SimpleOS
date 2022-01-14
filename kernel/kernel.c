#include "screen.h"
#include "stdio.h"
#include "interrupt.h"
#include "dsctbl.h"
#include "ports.h"
#include "fifo.h"

struct mouse_desc {
    unsigned char buf[3], phase;
    int x, y, btn;
    int mx;
    int my;
};
int mouse_decode(struct mouse_desc *mdec, unsigned char dat) {
    if (mdec->phase == 0) {
        if (dat == 0xfa) {
            mdec->phase = 1;
        }
        return 0;
    }
    if (mdec->phase == 1) {
        if ((dat & 0xc8) == 0x08) {
            mdec->buf[0] = dat;
            mdec->phase = 2;
        }
        return 0;
    }
    if (mdec->phase == 2) {
        mdec->buf[1] = dat;
        mdec->phase = 3;
        return 0;
    }
    if (mdec->phase == 3) {
        mdec->buf[2] = dat;
        mdec->phase = 1;
        mdec->btn = mdec->buf[0] & 0x07;
        mdec->x = mdec->buf[1];
        mdec->y = mdec->buf[2];

        if (mdec->buf[0] & 0x10) {
            mdec->x |= 0xffffff00;
        }
        if (mdec->buf[0] & 0x20) {
            mdec->y |= 0xffffff00;
        }
        mdec->y = -mdec->y;
        if (mdec->mx + mdec->x < 0) {
            mdec->mx = 0;
        } else if (mdec->mx + mdec->x > SCREEN_WIDTH) {
            mdec->mx = SCREEN_WIDTH;
        } else {
            mdec->mx += mdec->x;
        }
        if (mdec->my + mdec->y < 0) {
            mdec->my = 0;
        } else if (mdec->my + mdec->y > SCREEN_HEIGHT) {
            mdec->my = SCREEN_HEIGHT;
        } else {
            mdec->my += mdec->y;
        }

        return 1;
    }
    return -1;
}

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
