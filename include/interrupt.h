#include "ports.h"
#include "screen.h"
#include "fifo.h"

#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_

#define PIC0_ICW1 0x0020
#define PIC0_OCW2 0x0020
#define PIC0_IMR  0x0021
#define PIC0_ICW2 0x0021
#define PIC0_ICW3 0x0021
#define PIC0_ICW4 0x0021
#define PIC1_ICW1 0x00a0
#define PIC1_OCW2 0x00a0
#define PIC1_IMR  0x00a1
#define PIC1_ICW2 0x00a1
#define PIC1_ICW3 0x00a1
#define PIC1_ICW4 0x00a1

void init_pic(void);

void asm_int_handler21(void);
void asm_int_handler27(void);
void asm_int_handler2c(void);


#define KEY_FIFO_BUF_SIZE 32
struct FIFO_BYTES fifo_key;
unsigned char key_buf[KEY_FIFO_BUF_SIZE];

#endif