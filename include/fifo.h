#ifndef _FIFO_H_
#define _FIFO_H_
struct FIFO_BYTES {
    char *buf;
    int buf_size;
    int head;
    int tail;
    int count;
};
void fifo_bytes_init(struct FIFO_BYTES *fifo, int size, char *buf);
int fifo_bytes_put(struct FIFO_BYTES *fifo, unsigned char data);
int fifo_bytes_get(struct FIFO_BYTES *fifo, unsigned char *data);
int fifo_bytes_count(struct FIFO_BYTES *fifo);

#endif
