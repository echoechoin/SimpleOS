#include "dsctbl.h"

// 设置一个段描述符
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar) {
    if (limit > 0xfffff) {
        ar |= 0x8000; // G_bit = 1
        limit /= 0x1000;
    }
        
    sd->limit_low    = limit & 0xffff;
    sd->base_low     = base & 0xffff;
    sd->base_mid     = (base >> 16) & 0xff;
    sd->access_right = ar & 0xff;
    sd->limit_high   = ((limit >> 16) & 0x0f) | ((ar >> 8) | 0xf0);
    sd->base_high    = (base >> 24) & 0xff;
}

void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar) {
    gd->offset_low   = offset & 0xffff;
    gd->selector     = selector;
    gd->dw_count     = (ar >> 8) & 0xff;
    gd->access_right = ar & 0xff;
    gd->offset_high  = (offset >> 16) & 0xffff;
}

void load_gdtr(int limit, int addr) {
    char gdtr[6];
    gdtr[0] = limit & 0xff;
    gdtr[1] = (limit >> 8) & 0xff;
    gdtr[2] = addr & 0xff;
    gdtr[3] = (addr >> 8) & 0xff;
    gdtr[4] = (addr >> 16) & 0xff;
    gdtr[5] = (addr >> 24) & 0xff;
    __asm__ __volatile__ ("lgdt (%0)" : : "r" (gdtr));
}

void load_idtr(int limit, int addr) {
    char idtr[6];
    idtr[0] = limit & 0xff;
    idtr[1] = (limit >> 8) & 0xff;
    idtr[2] = addr & 0xff;
    idtr[3] = (addr >> 8) & 0xff;
    idtr[4] = (addr >> 16) & 0xff;
    idtr[5] = (addr >> 24) & 0xff;
    __asm__ __volatile__ ("lidt (%0)" : : "r" (idtr));
}

void init_gdt(void) {
    int i ;
    struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)ADDR_GDT;

    // 初始化GGDT
    for (i = 0; i < LIMIT_GDT / 8; i++) {
        set_segmdesc(gdt + i, 0, 0, 0);
    }
    set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, AR_DATA32_RW);
    set_segmdesc(gdt + 2, LIMIT_KERNEL, ADDR_KERNEL, AR_CODE32_ER);
    load_gdtr(LIMIT_GDT, ADDR_GDT);
}

void init_idt(void) {
    struct GATE_DESCRIPTOR *idt = (struct GATE_DESCRIPTOR *)ADDR_IDT;
    
    for (int i = 0; i <= LIMIT_IDT / 8; i++) {
        set_gatedesc(idt + i, 0, 0, 0);
    }
    set_gatedesc(idt + 0x20, (int)asm_int_handler20, 2 * 8, AR_INTGATE32);
    set_gatedesc(idt + 0x21, (int)asm_int_handler21, 2 * 8, AR_INTGATE32);
    set_gatedesc(idt + 0x27, (int)asm_int_handler27, 2 * 8, AR_INTGATE32);
    set_gatedesc(idt + 0x2c, (int)asm_int_handler2c, 2 * 8, AR_INTGATE32);
    load_idtr(LIMIT_IDT, ADDR_IDT); 
}