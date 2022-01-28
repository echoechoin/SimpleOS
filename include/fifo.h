#ifndef _FIFO_H_
#define _FIFO_H_
struct FIFO8 {
    char *buf;
    int buf_size;
    int head;
    int tail;
    int count;
};

struct FIFO32 {
    int *buf;
    int buf_size;
    int head;
    int tail;
    int count;
};

void fifo8_init(struct FIFO8 *fifo, int size, char *buf);
int fifo8_put(struct FIFO8 *fifo, unsigned char data);
int fifo8_get(struct FIFO8 *fifo, unsigned char *data);
int fifo8_count(struct FIFO8 *fifo);

void fifo32_init(struct FIFO32 *fifo, int size, int *buf);
int fifo32_put(struct FIFO32 *fifo, int data);
int fifo32_get(struct FIFO32 *fifo, int *data);
int fifo32_count(struct FIFO32 *fifo);


#endif
