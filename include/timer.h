#ifndef _TIMER_H_
#define _TIMER_H_

#include "fifo.h"
#include "ports.h"
#include "stdio.h"
#define PIT_CTRL 0x0043
#define PIT_CNT0 0x0040

#define MAX_TIMER 500

#define TIMER_FLAGS_ALLOC 1
#define TIMER_FLAGS_USING 2

struct TIMER {
    unsigned int timeout;
    unsigned int flags;
    struct FIFO32 *fifo;
    int data;
    struct TIMER *next;
};

struct TIMERCTL {
    unsigned int count;
    unsigned int next_time;
    struct TIMER *t0;
    struct TIMER timer0[MAX_TIMER];
};

void init_pit();
struct TIMER *timer_alloc();
void timer_free(struct TIMER *timer);
void timer_init(struct TIMER *timer, struct FIFO32 *fifo,int data);
void timer_settime(struct TIMER *timer, unsigned int timeout);
#endif