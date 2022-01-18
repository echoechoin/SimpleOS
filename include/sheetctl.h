#define MAX_SHEETS 256
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
};

struct SHTCTL
{
    unsigned char *vram;              // 显存地址
    int xsize;                        // 屏幕的宽度
    int ysize;                        // 屏幕的高度
    int top;
    struct SHEET *sheets[MAX_SHEETS];
    struct SHEET sheets0[MAX_SHEETS];
};


struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize);