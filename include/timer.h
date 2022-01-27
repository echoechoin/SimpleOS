#ifndef _TIMER_H_
#define _TIMER_H_

#include "fifo.h"
#include "ports.h"
#define PIT_CTRL 0x0043
#define PIT_CNT0 0x0040

struct TIMECTL {
    unsigned int count;
    unsigned int timeout;
    struct FIFO_BYTES *fifo;
    unsigned char data;
};

void init_pit();
void set_timer(unsigned int timeout, struct FIFO_BYTES *fifo, unsigned char data);
#endif