#include "screen.h"
#include "stdio.h"
#include "interrupt.h"
#include "dsctbl.h"
#include "ports.h"
#include "fifo.h"
#include "memory.h"
#include "sheetctl.h"
#include "window.h"
#include "timer.h"


void init_mem(struct MEMMAN *memman)
{
    char s[25] = {0};
    unsigned memtotal = memtest(0x00400000, 0xbfffffff);
    memman_init(memman);
    memman_free(memman, 0x00010000, 0x0009e000);
    memman_free(memman, 0x00400000, memtotal - 0x00400000);
}


void main() {
    unsigned char data;
    char s[25] = {0};
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
    struct SHTCTL *shtctl;
    struct SHEET *sht_back, *sht_mouse, *sht_win;
    unsigned char *buf_back, buf_mouse[256], *buf_win;
    struct FIFO_BYTES fifo_timer;
    char timerbuf[8];

    extern struct TIMECTL timerctl;
    unsigned char count = 0;

    init_gdt();
    init_idt();
    init_pic();
    init_palette();
    init_keyboard();
    init_mouse();
    init_pit();

    fifo_bytes_init(&fifo_key, KEY_FIFO_BUF_SIZE, key_buf);
    fifo_bytes_init(&fifo_mouse, MOUSE_FIFO_BUF_SIZE, mouse_buf);
    fifo_bytes_init(&fifo_timer, sizeof(timerbuf), timerbuf);

    _io_sti();
    port_byte_out(PIC0_IMR, 0xf8); // 开放PIT\PIC1\键盘中断
    port_byte_out(PIC1_IMR, 0xef); // 开放鼠标中断

    init_mem(memman);

    struct mouse_desc md = {
        .mx = SCREEN_WIDTH / 2,
        .my = SCREEN_HEIGHT / 2,
    };

    set_timer(100, &fifo_timer, 0);

/**************图层管理*************/
    shtctl = shtctl_init(memman, (unsigned char *)VGA_ADDRESS, SCREEN_WIDTH, SCREEN_HEIGHT);
    
    buf_back =(unsigned char *)memman_alloc_4k(memman, SCREEN_WIDTH * SCREEN_HEIGHT);
    buf_win = (unsigned char *)memman_alloc_4k(memman, 160 * 68);

    // 创建图层
    sht_back = sheet_alloc(shtctl);
    sht_mouse = sheet_alloc(shtctl);
    sht_win = sheet_alloc(shtctl);

    // 初始化buffer中的内容
    init_screen(buf_back, SCREEN_WIDTH, SCREEN_HEIGHT);
    init_mouse_cursor(buf_mouse, 99);
    init_window(buf_win, 160, 68, "window");

    // 设置图层的buffer和size
    sheet_setbuf(sht_back, buf_back, SCREEN_WIDTH, SCREEN_HEIGHT, -1);                               // 没有透明色
    sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99); // 透明色号99
    sheet_setbuf(sht_win, buf_win, 160, 68, -1);

    // 移动图层位置并刷新图层
    draw_string(buf_back, SCREEN_WIDTH, COL8_DARK_GREY, 10, SCREEN_HEIGHT -22, "start");
    sheet_slide(sht_back, 0, 0);
    sheet_slide(sht_mouse, md.mx, md.my);
    sheet_slide(sht_win, 80, 72);

    // 修改图层高度并刷新图层
    sheet_updown(sht_back, 0);
    sheet_updown(sht_win, 2);
    sheet_updown(sht_mouse, 2);
    
    // 修改并刷新图层
    sprintf(s, "count: %d", 0);
    draw_string(buf_win, 160, COL8_BLACK, 2, 24, s);
    sheet_refresh(sht_back, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    for (;;) {
        if (fifo_bytes_count(&fifo_key) + fifo_bytes_count(&fifo_mouse) + fifo_bytes_count(&fifo_timer) == 0) {
            continue;
        }
        int i = 0;
        if (fifo_bytes_count(&fifo_key) != 0) {
            i = fifo_bytes_get(&fifo_key, &data);
            sprintf(s, "key: %02x", data);
            draw_rectangle(buf_back, SCREEN_WIDTH, COL8_WHITE, 0, 48, 80, 64);
            draw_string(buf_back, SCREEN_WIDTH, COL8_BLACK, 0, 48, s);
            sheet_refresh(sht_back, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        } else if (fifo_bytes_count(&fifo_mouse) != 0) {
            i = fifo_bytes_get(&fifo_mouse, &data);
            if (mouse_decode(&md, data) != 0) {
                sprintf(s, "mouse: %d %d %d", md.x, md.y, md.btn);
                draw_rectangle(buf_back, SCREEN_WIDTH, COL8_WHITE, 0, 64, 200, 80);
                draw_string(buf_back, SCREEN_WIDTH, COL8_BLACK, 0, 64, s);
                sheet_refresh(sht_back, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
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
                sheet_slide(sht_mouse, md.mx, md.my);
            }
        } else if (fifo_bytes_count(&fifo_timer) != 0) {
            i = fifo_bytes_get(&fifo_timer, &data);
            count++;
            sprintf(s, "count: %d", count);
            draw_rectangle(buf_win, 160, COL8_LIGHT_GREY,2,24,80,40);
            draw_string(buf_win, 160, COL8_BLACK, 2, 24, s);
            sheet_refresh(sht_win, 0, 0, 160, 68);
            set_timer(100, &fifo_timer, 0);
        }
        
    }
}
