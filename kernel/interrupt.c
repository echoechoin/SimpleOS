#include "interrupt.h"

static void wait_KBC_sendready(void);
extern struct TIMERCTL timerctl;
void init_pic(void) {
    // IMR：中断屏蔽寄存器
    // 屏蔽PIC0和PIC1的所有中断
    port_byte_out(PIC0_IMR, 0xff);
    port_byte_out(PIC1_IMR, 0xff);

    //PIC1使用端口0x20接收命令，0x21接收数据。PIC2使用端口0xA0接收指令，0xA1接收数据。
    port_byte_out(PIC0_ICW1, 0x11);    // 设置为边沿触发模式 
    port_byte_out(PIC0_ICW2, 0x20);    // IRQ0-7由INT20-27接收
    port_byte_out(PIC0_ICW3, 1 << 2);  // PIC1由IRQ2接收
    port_byte_out(PIC0_ICW4, 0x01);    // 无缓冲区模式

    port_byte_out(PIC1_ICW1, 0x11);    // 设置为边沿触发模式
    port_byte_out(PIC1_ICW2, 0x28);    // IRQ8-15由INT28-2f接收
    port_byte_out(PIC1_ICW3, 2);       // PIC1由IRQ2接收
    port_byte_out(PIC1_ICW4, 0x01);    // 无缓冲区模式

    port_byte_out(PIC0_IMR, 0xfb);     // PIC1以外的所有中断都屏蔽
    port_byte_out(PIC1_IMR, 0xff);     // 全部屏蔽
}

static void wait_KBC_sendready(void) {
    for (;;) {
        if ((port_byte_in(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
            break;
        }
    }
    return;
}

void init_keyboard(struct FIFO32 *fifo, int data0) {
    fifo_key = fifo;
    data_key = data0;
    // 初始化键盘控制电路
    wait_KBC_sendready();
    port_byte_out(PORT_KEYCMD, KEYCMD_WRITE_MODE);
    wait_KBC_sendready();
    port_byte_out(PORT_KEYDAT, KBC_MODE);
    return;
}

void init_mouse(struct FIFO32 *fifo, int data0, struct mouse_desc *mdec) {
    fifo_mouse = fifo;
    data_mouse = data0;
    mdec->phase = 0;
    wait_KBC_sendready();
    port_byte_out(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
    wait_KBC_sendready();
    port_byte_out(PORT_KEYDAT, MOUSECMD_ENABLE);
    return;
}

int mouse_decode(struct mouse_desc *mdec, unsigned char dat, int x, int y) {
    dat = dat - 512;
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
        } else if (mdec->mx + mdec->x > x) {
            mdec->mx = x;
        } else {
            mdec->mx += mdec->x;
        }
        if (mdec->my + mdec->y < 0) {
            mdec->my = 0;
        } else if (mdec->my + mdec->y > y) {
            mdec->my = y;
        } else {
            mdec->my += mdec->y;
        }

        return 1;
    }
    return -1;
}

// IRQ0: 时钟中断
void int_handler20(void) {
    port_byte_out(PIC0_OCW2, 0x60); // 接收IRQ-00信号通知PIC
    timerctl.count++;
    if (timerctl.next_time > timerctl.count) {
        return;
    }
    char ts = 0;
    struct TIMER *timer = timerctl.t0;
    for (;;) {
        // 这里获取到了链表最前面的且没有超时的定时器
        if (timer->timeout > timerctl.count) {
            break;
        }

        // 处理超时了的定时器
        timer->flags = TIMER_FLAGS_ALLOC;
        if (timer != task_timer) {
            fifo32_put(timer->fifo, timer->data);
        } else {
            ts = 1;
        }
        timer = timer->next;
    }

    // 将链表最前面的且没有超时的定时器作为链表的第一个定时器
    timerctl.t0 = timer;
    timerctl.next_time = timerctl.t0->timeout;
    if (ts != 0) {
        task_switch();
    }
}

// IRQ1: 键盘中断
void int_handler21(void) {
    int data;
    port_byte_out(PIC0_OCW2, 0x61); // 通知PIC IRQ-1的受理已经完成
    data = (int)port_byte_in(0x60);
    fifo32_put(fifo_key, data + data_key);
    return;
}

// IRQ7: 
void int_handler27(void) {
    port_byte_out(PIC0_OCW2, 0x67);
    return;
}

// IRQ12: 鼠标中断
void int_handler2c(void) {
    int data;
    port_byte_out(PIC1_OCW2, 0x64); // 通知PIC1 IRQ-12的受理已经完成
    port_byte_out(PIC0_OCW2, 0x62); // 通知PIC0 IRQ-02的受理已经完成
    data = (int)port_byte_in(0x60);
    fifo32_put(fifo_mouse, data + data_mouse);
    return;
}

