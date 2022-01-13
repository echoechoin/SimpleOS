#include "interrupt.h"

static void wait_KBC_sendready(void);

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

void init_keyboard(void) {
    // 初始化键盘控制电路
    wait_KBC_sendready();
    port_byte_out(PORT_KEYCMD, KEYCMD_WRITE_MODE);
    wait_KBC_sendready();
    port_byte_out(PORT_KEYDAT, KBC_MODE);
    return;
}

void init_mouse(void) {
    wait_KBC_sendready();
    port_byte_out(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
    wait_KBC_sendready();
    port_byte_out(PORT_KEYDAT, MOUSECMD_ENABLE);
    return;
}

// IRQ1: 键盘中断
void int_handler21(void) {
    unsigned char data;
    data = port_byte_in(0x60);
    fifo_bytes_put(&fifo_key, data);
    port_byte_out(PIC0_OCW2, 0x61); // 通知PIC IRQ-1的受理已经完成
    return;
}

// IRQ7: 
void int_handler27(void) {
    port_byte_out(PIC0_OCW2, 0x67);
    return;
}

// IRQ12: 鼠标中断
void int_handler2c(void) {
    unsigned char data;
    data = port_byte_in(0x60);
    fifo_bytes_put(&fifo_mouse, data);
    port_byte_out(PIC1_OCW2, 0x64); // 通知PIC1 IRQ-12的受理已经完成
    port_byte_out(PIC0_OCW2, 0x62); // 通知PIC0 IRQ-02的受理已经完成
    return;
}