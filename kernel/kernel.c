#include "screen.h"
#include "stdio.h"
#include "interrupt.h"
#include "dsctbl.h"
#include "ports.h"

void main() {
    char buf_mouse[256];
    char s[25] = {0};
    init_gdt();
    init_idt();
    init_pic();
    init_palette();
    init_screen(SCREEN_WIDTH, SCREEN_HEIGHT);
    init_mouse(buf_mouse, COL8_WHITE);

    port_byte_out(PIC0_IMR, 0xf9); // 开放PIC1以及键盘中断
    port_byte_out(PIC1_IMR, 0xef); // 开放鼠标中断
    _io_sti();

    sprintf(s, "vga_address: 0x%x", VGA_ADDRESS);

    draw_mouse(100,100,buf_mouse);
    draw_string(COL8_RED, 0, 0, s);

    for (;;) {
        __asm("hlt");
    }
}
