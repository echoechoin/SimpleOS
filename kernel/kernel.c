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

void main() {
    struct boot_info *BOOT_INFO = (struct boot_info *) BOOT_INFO_ADDR;
    int data;
    char s[25] = {0};
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
    struct SHTCTL *shtctl;
    struct SHEET *sht_back, *sht_mouse, *sht_win;
    unsigned char *buf_back, buf_mouse[256], *buf_win;
    struct FIFO32 fifo;
    int fifobuf[128];
    struct TIMER *timer1, *timer2, *timer3;
    char timerbuf[8];
    struct mouse_desc md = {0};
    extern struct TIMERCTL timerctl;
    unsigned char count = 0;

    // 初始化fifo
    fifo32_init(&fifo, 128, fifobuf);

    // 初始化全局描述符
    init_gdt();

    // 初始化中断描述符
    init_idt();

    // 初始化可编程中断控制器
    init_pic();

    // 初始化颜色版
    init_palette();

    // 初始化键盘控制电路以及指定fifo
    init_keyboard(&fifo, 256);

    // 初始化鼠标控制电路以及指定fifo
    init_mouse(&fifo, 512, &md);
    md.mx = BOOT_INFO->scrnx / 2;
    md.my = BOOT_INFO->scrny / 2;

    // 初始化时钟
    init_pit();

    // 开放PIT\PIC1\键盘中断
    port_byte_out(PIC0_IMR, 0xf8);

    // 开放鼠标中断
    port_byte_out(PIC1_IMR, 0xef);

    // 开放所有中断
    _io_sti();

    // 初始化定时器
    timer1 = timer_alloc();
    timer2 = timer_alloc();
    timer3 = timer_alloc();

    timer_init(timer1, &fifo, 1);
    timer_init(timer2, &fifo, 2);
    timer_init(timer3, &fifo, 3);

    timer_settime(timer1, 100);
    timer_settime(timer2, 200);
    timer_settime(timer3, 500);

    // 初始化内存
    unsigned memtotal = memtest(0x00400000, 0xbfffffff);
    memman_init(memman);
    memman_free(memman, 0x00010000, 0x0009e000);
    memman_free(memman, 0x00400000, memtotal - 0x00400000);

    // 图层管理
    shtctl = shtctl_init(memman, (unsigned char *)BOOT_INFO->vram, BOOT_INFO->scrnx, BOOT_INFO->scrny);
    
    buf_back =(unsigned char *)memman_alloc_4k(memman, BOOT_INFO->scrnx * BOOT_INFO->scrny);
    buf_win = (unsigned char *)memman_alloc_4k(memman, 160 * 68);

    // > 创建图层
    sht_back = sheet_alloc(shtctl);
    sht_mouse = sheet_alloc(shtctl);
    sht_win = sheet_alloc(shtctl);

    // > 初始化buffer中的内容
    init_screen(buf_back, BOOT_INFO->scrnx, BOOT_INFO->scrny);
    init_mouse_cursor(buf_mouse, 99);
    init_window(buf_win, 160, 68, "window");

    // > 设置图层的buffer和size
    sheet_setbuf(sht_back, buf_back, BOOT_INFO->scrnx, BOOT_INFO->scrny, -1);                               // 没有透明色
    sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99); // 透明色号99
    sheet_setbuf(sht_win, buf_win, 160, 68, -1);

    // > 移动图层位置并刷新图层
    draw_string(buf_back, BOOT_INFO->scrnx, COL8_DARK_GREY, 10, BOOT_INFO->scrny -22, "start");
    sheet_slide(sht_back, 0, 0);
    sheet_slide(sht_mouse, md.mx, md.my);
    sheet_slide(sht_win, 160, 120);

    // > 修改图层高度并刷新图层
    sheet_updown(sht_back, 0);
    sheet_updown(sht_win, 2);
    sheet_updown(sht_mouse, 2);
    
    // > 修改并刷新图层
    sprintf(s, "memory %dMB", memtotal / (1024 * 1024));
    draw_background_and_string(buf_back, BOOT_INFO->scrnx, COL8_BLACK, COL8_WHITE, 0, 0, s);
    sprintf(s, "memory free %dKB", memman_total(memman) / 1024);
    draw_background_and_string(buf_back, BOOT_INFO->scrnx, COL8_BLACK, COL8_WHITE, 0, 16, s);
    sprintf(s, "count: %d", 0);
    draw_string(buf_win, 160, COL8_BLACK, 2, 24, s);

    sprintf(s, "scrnx: %d, scrny: %d", BOOT_INFO->scrnx, BOOT_INFO->scrny);
    draw_background_and_string(buf_back, BOOT_INFO->scrnx, COL8_BLACK, COL8_WHITE, 0, 80, s);
    sprintf(s, "vram: %x", BOOT_INFO->vram);
    draw_background_and_string(buf_back, BOOT_INFO->scrnx, COL8_BLACK, COL8_WHITE, 0, 96, s);

    draw_background_and_string(buf_back, BOOT_INFO->scrnx, COL8_BLACK, COL8_WHITE, 0, 32, "key:");
    draw_background_and_string(buf_back, BOOT_INFO->scrnx, COL8_BLACK, COL8_WHITE, 0, 48, "mouse:");
    sheet_refresh(sht_back, 0, 0, BOOT_INFO->scrnx, BOOT_INFO->scrny);
    for (;;) {
        _io_cli();
        if (fifo32_count(&fifo) == 0) {
            _io_sti();
            continue;
        }
        fifo32_get(&fifo, &data);

        // 判断是否是键盘中断
        if (data >= 256 && data <= 511) {
            sprintf(s, "key: %02x", data);
            draw_background_and_string(buf_back, BOOT_INFO->scrnx, COL8_BLACK, COL8_WHITE, 0, 32, s);
            sheet_refresh(sht_back, 0, 0, 200, 200);
        // 判断是否是鼠标中断
        } else if (512 <= data && data <= 767) {
            if (mouse_decode(&md, data, BOOT_INFO->scrnx, BOOT_INFO->scrny) != 0) {
                sprintf(s, "mouse: %d %d %d       ", md.x, md.y, md.btn);
                draw_background_and_string(buf_back, BOOT_INFO->scrnx, COL8_BLACK, COL8_WHITE, 0, 48, s);
                sheet_refresh(sht_back, 0, 0, 200, 200);
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
                if (x1 > BOOT_INFO->scrnx) {
                    x1 = BOOT_INFO->scrnx;
                }
                if (y1 > BOOT_INFO->scrny) {
                    y1 = BOOT_INFO->scrny;
                }
                sheet_slide(sht_mouse, md.mx, md.my);
            }
        // 判断是否为定时器中断
        } else if ( 1 <=data && data <= 3) {
            sprintf(s, "timer: %d", data);
            draw_background_and_string(buf_win, 160, COL8_BLACK, COL8_LIGHT_GREY, 2, 24, s);
            sheet_refresh(sht_win, 0, 0, 160, 68);
            if (data == 1) {
                timer_settime(timer1, 100);
            } else if (data == 2) {
                timer_settime(timer2, 200);
            } else if (data == 3) {
                timer_settime(timer3, 500);
            }
        } else {
            sprintf(s, "data unknown: %d", data);
            draw_background_and_string(buf_back, BOOT_INFO->scrnx, COL8_BLACK, COL8_WHITE, 0, 64, s);
            sheet_refresh(sht_back, 0, 0, 200, 200);
        }
        
    }
}
