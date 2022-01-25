#include "sheetctl.h"

static void sheet_refreshsub(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0);
// 初始化图层控制器 主要记录显存位置和size
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
        shtctl->sheets0[i].ctl = shtctl;
    }
    return shtctl;
}

// 创建图层
struct SHEET *sheet_alloc(struct SHTCTL *ctl) {
    struct SHEET *sht;
    for (int i = 0; i < MAX_SHEETS; i++) {
        if (ctl->sheets0[i].flags == 0) {
        sht = &ctl->sheets0[i];
        sht->flags = SHEET_USE; // 标记为正在使用
        sht->height = -1;       // 隐藏
        return sht;
        }
    }

    return NULL;
}

// 设置图层的buffer和size
void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize,
                  int col_inv) {
    sht->buf = buf;
    sht->bxsize = xsize;
    sht->bysize = ysize;
    sht->col_inv = col_inv;
}

// 上下移动图层并刷新图层
void sheet_updown(struct SHEET *sht, int height) {
    struct SHTCTL *ctl = sht->ctl;
    int h, old = sht->height;

    // 如果指定的高度过高或过低，则进行修正
    if (height > ctl->top + 1) {
        height = ctl->top + 1;
    }

    if (height < -1) {
        height = -1;
    }

    sht->height = height; // 设定高度

    // 下面主要是进行sheets[]的重新排列
    if (old > height) {
        // 比以前低
        if (height >= 0) {
        // 把中间的往上提
        for (h = old; h > height; h--) {
            ctl->sheets[h] = ctl->sheets[h - 1];
            ctl->sheets[h]->height = h;
        }
        ctl->sheets[height] = sht;
        sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize,
                        sht->vy0 + sht->bysize, height + 1);
        } else {
        // 隐藏
        if (ctl->top > old) {
            // 把上面的降下来
            for (h = old; h < ctl->top; h++) {
            ctl->sheets[h] = ctl->sheets[h + 1];
            ctl->sheets[h]->height = h;
            }
        }
        ctl->top--; // 由于显示中的图层减少了一个，所以最上面的图层高度下降
        }
        sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize,
                        sht->vy0 + sht->bysize, 0); // 按新图层的信息重新绘制画面
    } else if (old < height) {
        // 比以前高
        if (old >= 0) {
        // 把中间的拉下去
        for (h = old; h < height; h++) {
            ctl->sheets[h] = ctl->sheets[h + 1];
            ctl->sheets[h]->height = h;
        }
        ctl->sheets[height] = sht;
        } else {
        // 由隐藏状态转为显示状态
        // 将已在上面的提上来
        for (h = ctl->top; h >= height; h--) {
            ctl->sheets[h + 1] = ctl->sheets[h];
            ctl->sheets[h + 1]->height = h + 1;
        }
        ctl->sheets[height] = sht;
        ctl->top++; // 由于已显示的图层增加了1个，所以最上面的图层高度增加
        }
        sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize,
                        sht->vy0 + sht->bysize, height); // 按新图层的信息重新绘制画面
    }
}

// 刷新某个区域
static void sheet_refreshsub(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0) {
    unsigned char *buf, *vram = ctl->vram;
    struct SHEET *sht;

    for (int h = h0; h <= ctl->top; h++) {
        sht = ctl->sheets[h];
        buf = sht->buf;
        // 使用vx0 ~ vy1，对bx0 ~ by1进行倒推
        // 以下坐标为相对于图层的坐标
        int bx0 = vx0 - sht->vx0;
        int by0 = vy0 - sht->vy0;
        int bx1 = vx1 - sht->vx0;
        int by1 = vy1 - sht->vy0;

        if (bx0 < 0) {
        bx0 = 0;
        }
        if (by0 < 0) {
        by0 = 0;
        }
        if (bx1 > sht->bxsize) {
        bx1 = sht->bxsize;
        }
        if (by1 > sht->bysize) {
        by1 = sht->bysize;
        }

        for (int by = by0; by < by1; by++) {
        // 计算出绝对坐标
        int vy = sht->vy0 + by;
        for (int bx = bx0; bx < bx1; bx++) {
            int vx = sht->vx0 + bx;
            unsigned char c = buf[by * sht->bxsize + bx];
            if (c != sht->col_inv) {
            draw_pixel(vram, ctl->xsize, c, vx, vy);
            }
        }
        }
    }
}

// 刷新图层
void sheet_refresh(struct SHEET *sht, int bx0, int by0,
                   int bx1, int by1) {
    struct SHTCTL *ctl = sht->ctl;
    if (sht->height >= 0) {
        // 如果正在显示，则按新图层的信息刷新画面
        sheet_refreshsub(ctl, sht->vx0 + bx0, sht->vy0 + by0, sht->vx0 + bx1,
                        sht->vy0 + by1, sht->height);
    }
}

// 移动图层
void sheet_slide(struct SHEET *sht, int vx0, int vy0) {
    struct SHTCTL *ctl = sht->ctl;
    int old_vx0 = sht->vx0, old_vy0 = sht->vy0;

    sht->vx0 = vx0;
    sht->vy0 = vy0;

    if (sht->height >= 0) {
        // 如果正在显示，则按新图层的信息刷新画面
        sheet_refreshsub(ctl, old_vx0, old_vy0, old_vx0 + sht->bxsize,
                        old_vy0 + sht->bysize, 0);
        sheet_refreshsub(ctl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize, sht->height);
    }
}

// 释放图层
void sheet_free(struct SHEET *sht) {
    struct SHTCTL *ctl = sht->ctl;
    if (sht->height >= 0) {
        sheet_updown(sht, -1); // 如果处于显示状态，则先设定为隐藏
    }

    sht->flags = 0; // 未使用标志
}
