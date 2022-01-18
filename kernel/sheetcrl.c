#include "sheetctl.h"

struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize) {
    struct SHTCTL *shtctl;
    int i;
    shtctl = (struct SHTCTL *) memman_alloc_4k(memman, sizeof(struct SHTCTL));
    if (shtctl == 0) {
        return 0;
    }
    shtctl->vram = vram;
    shtctl->xsize = xsize;
    shtctl->ysize = ysize;
    shtctl->top = -1;
    for (i = 0; i < MAX_SHEETS; i++) {
        shtctl->sheets0[i].flags = 0;
    }
    return shtctl;
}