#ifndef _FIFO_H_
#define _FIFO_H_
#include "task.h"

struct FIFO32 {
    int *buf;
    int buf_size;
    int head;
    int tail;
    int count;
    struct TASK *task;
};

void fifo32_init(struct FIFO32 *fifo, int size, int *buf, struct TASK *task);
int fifo32_put(struct FIFO32 *fifo, int data);
int fifo32_get(struct FIFO32 *fifo, int *data);
int fifo32_count(struct FIFO32 *fifo);


#endif
