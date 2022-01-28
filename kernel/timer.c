#include "timer.h"
struct TIMERCTL timerctl = {0};

// 设置时钟中断频率为100HZ. 即每10ms产生一次中断
void init_pit(){
    port_byte_out(PIT_CTRL, 0x34);
    port_byte_out(PIT_CNT0, 0x9c);
    port_byte_out(PIT_CNT0, 0x2e);
    timerctl.count = 0;
    timerctl.next_time = 0xffffffff;

    for (int i = 0; i < MAX_TIMER; i++) {
        timerctl.timer0[i].flags = 0;
    }

    // 设置一个哨兵， 用于判断链表是否到达尽头
    struct TIMER *t = timer_alloc();
    t->timeout = 0xffffffff;
    t->flags = TIMER_FLAGS_USING;
    t->next = NULL;

    timerctl.count = 0;
    timerctl.t0 = t;
    timerctl.next_time = 0xffffffff;
}

struct TIMER *timer_alloc() 
{
    int i;
    for (i = 0; i < MAX_TIMER; i++) {
        if (timerctl.timer0[i].flags == 0) {
            timerctl.timer0[i].flags = TIMER_FLAGS_ALLOC;
            return &timerctl.timer0[i];
        }
    }
    return NULL;
}

void timer_free(struct TIMER *timer)
{
    timer->flags = 0;
}

void timer_init(struct TIMER *timer, struct FIFO32 *fifo, int data)
{
    timer->fifo = fifo;
    timer->data = data;
}

void timer_settime(struct TIMER *timer, unsigned int timeout)
{
    int eflags;
    struct TIMER *t, *s;

    timer->timeout = timeout + timerctl.count;
    timer->flags = TIMER_FLAGS_USING;

    eflags = _io_load_eflags();
    _io_cli();

    // 以下程序以timeout的值大小来从小到大排序链表：

    // 插入到链表的头部
    t = timerctl.t0;
    if (timer->timeout <= t->timeout) {
        timerctl.t0 = timer;
        timer->next = t;
        timerctl.next_time = timer->timeout;
        _io_restore_eflags(eflags);
        return;
    }

    // 插入到链表的中间
    for (;;) {
        s = t;
        t = t->next;

        if (timer->timeout <= t->timeout) {
            s->next = timer;
            timer->next = t;
            _io_restore_eflags(eflags);
            return;
        }
    }
}
