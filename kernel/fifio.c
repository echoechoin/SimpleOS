#include "fifo.h"

void fifo_bytes_init(struct FIFO_BYTES *fifo, int size, char *buf) {
    fifo->buf = buf;
    fifo->buf_size = size;
    fifo->head = 0;
    fifo->tail = 0;
    fifo->count = 0;
}

int fifo_bytes_put(struct FIFO_BYTES *fifo, unsigned char data) {
    if (fifo->count == fifo->buf_size) {
        return -1;
    }
    fifo->buf[fifo->head] = data;
    fifo->head = (fifo->head + 1) % fifo->buf_size;
    fifo->count++;
    return 0;
}

int fifo_bytes_get(struct FIFO_BYTES *fifo, unsigned char *data) {
    if (fifo->count == 0) {
        return -1;
    }
    *data = fifo->buf[fifo->tail];
    fifo->tail = (fifo->tail + 1) % fifo->buf_size;
    fifo->count--;
    return 0;
}

int fifo_bytes_count(struct FIFO_BYTES *fifo) {
    return fifo->count;
}