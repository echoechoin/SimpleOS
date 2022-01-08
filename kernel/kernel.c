#include "../drivers/screen.h"
#include "util.h"

void write_mem8(int addr, char data)
{
    *(char *)addr = data;
}

void main() {
    int i;
    for (i = 0xa0000; i < 0xaffff; i++) {
        write_mem8((char *)i, i & 0x0f);
    }

    for (;;);
}
