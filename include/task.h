#ifndef _TASK_H_
#define _TASK_H_
#include "timer.h"
#include "memory.h"
#include "dsctbl.h"

#define MAX_TASKS 1000
#define TASK_GDT0 3
struct TSS32 {
    int backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3;
    int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
    int es, cs, ss, ds, fs, gs;
    int ldtr, iomap;
};

struct TASK {
    int sel; // 存放GDT的编号
    int flags; // 判断任务是否在运行
    int priority; // 优先级
    struct TSS32 tss;
};

struct TASKCTL {
    struct TASK *tasks[MAX_TASKS];
    struct TASK tasks0[MAX_TASKS];
    int now;
    int running;
};

extern struct TIMER *task_timer;

void load_tr(int tr);
void far_jmp(int eip, int cs);

struct TASK *task_init(struct MEMMAN *memman);
struct TASK *task_alloc(void);
void task_sleep(struct TASK *task);
void task_run(struct TASK *task, int priority);
void task_switch(void);

#endif
