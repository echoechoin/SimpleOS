#ifndef _SHEETCLT_H_
#define _SHEETCLT_H_

#include "memory.h"
#include "stdio.h"
#include "screen.h"
#define MAX_SHEETS 256
#define SHEET_USE 1
struct SHEET
{
    unsigned char *buf;
    int bxsize;
    int bysize;  // 图层的大小
    int vx0;
    int vy0;     // 图层的位置
    int col_inv; // 透明色
    int height;  // 高度
    int flags;   // 图层的设定

    struct SHTCTL * ctl;
};

struct SHTCTL
{
    unsigned char *vram;              // 显存地址
    unsigned char *map;               // 图层映射表
    int xsize;                        // 屏幕的宽度
    int ysize;                        // 屏幕的高度
    int top;                          // 当前最上面的图层
    struct SHEET *sheets[MAX_SHEETS];
    struct SHEET sheets0[MAX_SHEETS];
};


struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize);
struct SHEET *sheet_alloc(struct SHTCTL *ctl);
void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv);
void sheet_updown(struct SHEET *sht, int height);
void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1);
void sheet_refreshmap(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0);
void sheet_refreshsub(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0, int h1) ;
void sheet_slide(struct SHEET *sht, int vx0, int vy0);
void sheet_free(struct SHEET *sht);
#endif