#include "fifo.h"

void fifo32_init(struct FIFO32 *fifo, int size, int *buf, struct TASK *task)
{
    fifo->buf = buf;
    fifo->buf_size = size;
    fifo->head = 0;
    fifo->tail = 0;
    fifo->count = 0;
    fifo->task = task;
}
int fifo32_put(struct FIFO32 *fifo, int data)
{
    if (fifo->count == fifo->buf_size) {
        return -1;
    }
    fifo->buf[fifo->head] = data;
    fifo->head = (fifo->head + 1) % fifo->buf_size;
    fifo->count++;
    if (fifo->task != NULL) {
        if (fifo->task->flags != 2) {
            task_run(fifo->task);
        }
    }
    return 0;
}
int fifo32_get(struct FIFO32 *fifo, int *data)
{
    if (fifo->count == 0) {
        return -1;
    }
    *data = fifo->buf[fifo->tail];
    fifo->tail = (fifo->tail + 1) % fifo->buf_size;
    fifo->count--;
    return 0;
}
int fifo32_count(struct FIFO32 *fifo)
{
    return fifo->count;
}