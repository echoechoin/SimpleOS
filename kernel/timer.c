#include "timer.h"

struct TIMECTL timerctl = {0};

// 设置时钟中断频率为100HZ. 即每10ms产生一次中断
void init_pit(){
    port_byte_out(PIT_CTRL, 0x34);
    port_byte_out(PIT_CNT0, 0x9c);
    port_byte_out(PIT_CNT0, 0x2e);
}

void set_timer(unsigned int timeout, struct FIFO_BYTES *fifo, unsigned char data) {
    int eflags;
    eflags = _io_load_eflags();
    _io_cli();
    timerctl.timeout = timeout;
    timerctl.data = data;
    timerctl.fifo = fifo;
    _io_restore_eflags(eflags);
}

