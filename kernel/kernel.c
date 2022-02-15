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
#include "keyboard.h"
#include "task.h"
#include "dsctbl.h"

void task_b_main(struct SHEET *sht_bak) {
    struct FIFO32 fifo;
    struct TIMER *timer_refresh;
    int i, fifobuf[128];

    fifo32_init(&fifo, 128, fifobuf, NULL);
    timer_refresh = timer_alloc();
    timer_init(timer_refresh, &fifo, 1);
    timer_settime(timer_refresh, 3);
    int data;
    int count = 0, count0 = 0;
    char s[20];
    for (;;) {
        count++;
        _io_cli();
        if (fifo32_count(&fifo) == 0) {
            _io_sti();
        } else {
            fifo32_get(&fifo, &data);
            _io_sti();
            if (data == 1){
                sprintf(s, "count: %d", count - count0);
                draw_string_with_refresh(sht_bak, 144, COL8_BLACK, COL8_WHITE, 4, 16 * 2, s);
                timer_settime(timer_refresh, 3);
                count0 = count;
            }
        }
    }
}

void main() {
    struct boot_info *BOOT_INFO = (struct boot_info *) BOOT_INFO_ADDR;
    int data;
    char s[100] = {0};
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
    struct SHTCTL *shtctl;
    struct SHEET *sht_back, *sht_mouse, *sht_win_b[3];
    unsigned char *buf_back, buf_mouse[256], *buf_win_b[3];
    struct FIFO32 fifo;
    int fifobuf[128];
    struct TIMER *timer1, *timer2;
    char timerbuf[8];
    struct mouse_desc md = {0};
    extern struct TIMERCTL timerctl;
    unsigned char count = 0;
    int cursor_x = 8;
    struct TASK *task_a, *task_b[3];
    struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADDR_GDT;
    int task2_esp;
    int i;

    // 初始化fifo
    fifo32_init(&fifo, 128, fifobuf, NULL);

    // 初始化全局描述符
    init_gdt();

    // 初始化中断描述符
    init_idt();

    // 初始化可编程中断控制器
    init_pic();

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
    timer_init(timer1, &fifo, 1);
    timer_settime(timer1, 100);

    // 初始化内存
    unsigned memtotal = memtest(0x00400000, 0xbfffffff);
    memman_init(memman);
    // memman_free(memman, 0x00010000, 0x0009e000);
    memman_free(memman, 0x00400000, memtotal - 0x00400000);


    // 图层管理
    init_palette(); // 初始化颜色版
    shtctl = shtctl_init(memman, (unsigned char *)BOOT_INFO->vram, BOOT_INFO->scrnx, BOOT_INFO->scrny);
    
    // > 初始化背景和鼠标图层
    buf_back =(unsigned char *)memman_alloc_4k(memman, BOOT_INFO->scrnx * BOOT_INFO->scrny);
    sht_back = sheet_alloc(shtctl);
    sht_mouse = sheet_alloc(shtctl);
    init_screen(buf_back, BOOT_INFO->scrnx, BOOT_INFO->scrny);
    init_mouse_cursor(buf_mouse, 99);
    sheet_setbuf(sht_back, buf_back, BOOT_INFO->scrnx, BOOT_INFO->scrny, -1);                               // 没有透明色
    sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
    sheet_slide(sht_back, 0, 0);
    sheet_slide(sht_mouse, md.mx, md.my);
    sheet_updown(sht_back, 0);
    sheet_updown(sht_mouse, 10);

    // 初始化多任务
    task_a = task_init(memman);
    fifo.task = task_a;

    // > 初始化sht_win_b
    for (i = 0; i < 3; i++) {
        sht_win_b[i] = sheet_alloc(shtctl);
        buf_win_b[i] = (unsigned char *)memman_alloc_4k(memman, 144 * 52);
        sheet_setbuf(sht_win_b[i], buf_win_b[i], 144, 52, -1);
        sprintf(s, "task_b%d", i + 1);
        init_window(buf_win_b[i], 144, 52, s);
        sheet_slide(sht_win_b[i], 288, 52 * i);
        sheet_updown(sht_win_b[i], i + 1);
        task_b[i] = task_alloc();
        task_b[i]->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 8;
        task_b[i]->tss.eip = (int) &task_b_main;
        task_b[i]->tss.cs = 2 * 8;
        task_b[i]->tss.ss = 1 * 8;
        task_b[i]->tss.ds = 1 * 8;
        task_b[i]->tss.es = 1 * 8;
        task_b[i]->tss.fs = 1 * 8;
        task_b[i]->tss.gs = 1 * 8;
        *((int *) (task_b[i]->tss.esp + 4)) = (int) sht_win_b[i];
        task_run(task_b[i], (i + 1));
    }
    
    sprintf(s, "memory %dMB", memtotal / (1024 * 1024));
    draw_string_with_refresh(sht_back, BOOT_INFO->scrnx, COL8_BLACK, COL8_WHITE, 0, 0, s);
    sprintf(s, "memory free %dKB", memman_total(memman) / 1024);
    draw_string_with_refresh(sht_back, BOOT_INFO->scrnx, COL8_BLACK, COL8_WHITE, 0, 16, s);

    for (;;) {
        _io_cli();
        if (fifo32_count(&fifo) == 0) {
            task_sleep(task_a);
            _io_sti();
            continue;
        }
        fifo32_get(&fifo, &data);

        // 判断是否是键盘中断
        if (data >= 256 && data <= 511) {
            sprintf(s, "key: %02x", data);
            draw_string_with_refresh(sht_back, BOOT_INFO->scrnx, COL8_BLACK, COL8_WHITE, 0, 16 * 7, s);

        // 判断是否是鼠标中断
        } else if (512 <= data && data <= 767) {
            if (mouse_decode(&md, data, BOOT_INFO->scrnx, BOOT_INFO->scrny) != 0) {
                sprintf(s, "mouse: %d %d %d       ", md.x, md.y, md.btn);
                draw_string_with_refresh(sht_back, BOOT_INFO->scrnx, COL8_BLACK, COL8_WHITE, 0, 16 * 8, s);
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
                if ((md.btn & 0x01) != 0) {
                    // sheet_slide(sht_text, x0, y0);
                }
            }
        // 判断是否为定时器中断
        } else if ( 1 <=data && data <= 10) {
            sprintf(s, "timer: %d", i++);
            draw_string_with_refresh(sht_back, BOOT_INFO->scrnx, COL8_BLACK, COL8_LIGHT_GREY, 0, 16 * 10, s);
            if (data == 1) {
                timer_settime(timer1, 100);
            }
            
        } else {
            sprintf(s, "data unknown: %d", data);
            draw_string_with_refresh(sht_back, BOOT_INFO->scrnx, COL8_BLACK, COL8_WHITE, 0, 16 * 9, s);
        }
        
    }
}
