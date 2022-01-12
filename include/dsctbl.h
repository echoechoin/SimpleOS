# include "interrupt.h"

#ifndef _DSCTBL_H_
#define _DSCTBL_H_

#define ADDR_GDT  0x00270000
#define LIMIT_GDT 0x0000ffff

#define ADDR_IDT  0x0026f800
#define LIMIT_IDT 0x000007ff

#define AR_DATA32_RW 0x4092
#define AR_CODE32_ER 0x409a
#define AR_INTGATE32 0x008e

#define ADDR_KERNEL 0x00280000
#define LIMIT_KERNEL 0x0007ffff

struct SEGMENT_DESCRIPTOR {
    short limit_low;
    short base_low;
    char  base_mid;
    char  access_right;
    char  limit_high;
    char  base_high;
};

struct GATE_DESCRIPTOR {
    short offset_low;
    short selector;
    char  dw_count;
    char  access_right;
    short offset_high;
};

void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar);

// void load_gdtr(int addr);
// void load_idtr(int addr);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);

void init_gdt(void);
void init_idt(void);

#endif

