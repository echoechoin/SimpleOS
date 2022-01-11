#include "screen.h"
#include "util.h"
#include "stdio.h"

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

void load_gdtr(int addr) {
    __asm__ __volatile__("lgdt %0":: "m"(addr));
}

void load_idtr(int addr) {
    __asm__ __volatile__("lidt %0":: "m"(addr));
}


#define ADDR_GDT  0x00270000
#define LIMIT_GDT 0x0000ffff

#define ADDR_IDT  0x0026f800
#define LIMIT_IDT 0x000007ff

void init_gdt(void) {
    int i ;
    struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)ADDR_GDT;

    // 初始化GGDT
    for (i = 0; i < LIMIT_GDT / 8; i++) {
        set_segmdesc(gdt + i, 0, 0, 0);
    }
    set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, 0x4092);
    set_segmdesc(gdt + 2, 0x0007ffff, 0x00280000, 0x4092);
    load_gdtr(0x270000);
}

// void init_idt(void) {
//     struct GATE_DESCRIPTOR *idt = (struct GATE_DESCRIPTOR *)0x268f00;
    
//     for (int i = 0; i <= LIMIT_IDT / 8; i++) {
//         set_gatedesc(idt + i, 0, 0, 0);
//     }
//     load_idtr(LIMIT_IDT, ADR_IDT);

//     set_gatedesc(idt + 0x21, (int)asm_int_handler21, 2 * 8, AR_INTGATE32);
//     set_gatedesc(idt + 0x27, (int)asm_int_handler27, 2 * 8, AR_INTGATE32);
//     set_gatedesc(idt + 0x2c, (int)asm_int_handler2c, 2 * 8, AR_INTGATE32);
// }

void main() {
    char buf_mouse[256];
    char s[25] = {0};
    init_gdt();
    init_palette();
    init_screen(SCREEN_WIDTH, SCREEN_HEIGHT);
    init_mouse(buf_mouse, COL8_WHITE);

    sprintf(s, "vga_address: 0x%x", VGA_ADDRESS);

    draw_mouse(100,100,buf_mouse);
    draw_string(COL8_RED, 0, 0, s);

    
    for (;;) {
        __asm("hlt");
    }
}
