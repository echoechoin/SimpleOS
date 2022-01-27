#include "ports.h"
#include "screen.h"
#include "fifo.h"
#include "timer.h"

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

#define PORT_KEYDAT 0x0060
#define PORT_KEYSTA 0x0064
#define PORT_KEYCMD 0x0064
#define KEYSTA_SEND_NOTREADY 0x02
#define KEYCMD_WRITE_MODE 0x60
#define KBC_MODE 0x47
#define KEYCMD_SENDTO_MOUSE 0xd4
#define MOUSECMD_ENABLE 0xf4

void init_keyboard(void);
void init_mouse(void);

void init_pic(void);
void asm_int_handler20(void);
void asm_int_handler21(void);
void asm_int_handler27(void);
void asm_int_handler2c(void);

// keyboard
#define KEY_FIFO_BUF_SIZE 32
struct FIFO_BYTES fifo_key;
unsigned char key_buf[KEY_FIFO_BUF_SIZE];


// mouse
#define MOUSE_FIFO_BUF_SIZE 32
struct mouse_desc {
    unsigned char buf[3], phase;
    int x, y, btn;
    int mx;
    int my;
};
struct FIFO_BYTES fifo_mouse;
unsigned char mouse_buf[MOUSE_FIFO_BUF_SIZE];


int mouse_decode(struct mouse_desc *mdec, unsigned char dat);


#endif