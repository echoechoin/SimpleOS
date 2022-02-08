#include "task.h"

struct TASKCTL *taskctl;
struct TIMER *task_timer;

struct TASK *task_init(struct MEMMAN *memman) {
    int i;
    struct TASK *task;
    struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADDR_GDT;
    taskctl = (struct TASKCTL *) memman_alloc_4k(memman, sizeof(struct TASKCTL));
    for (i = 0; i < MAX_TASKS; i++) {
        taskctl->tasks0[i].flags = 0;
        taskctl->tasks0[i].sel = (TASK_GDT0 + i) * 8;
        set_segmdesc(gdt + TASK_GDT0 + i, 103, (int) &taskctl->tasks0[i].tss, AR_TSS32);
    }
    task = task_alloc();
    task->flags = 2; // 初始化为第一个任务 2 表示running 1表示sleeping
    taskctl->running = 1;
    taskctl->now = 0;
    taskctl->tasks[0] = task;
    load_tr(task->sel);
    task_timer = timer_alloc();
    timer_settime(task_timer, 10);
    return task;
}

struct TASK *task_alloc(void) {
    int i;
    struct TASK *task;
    for(i = 0; i < MAX_TASKS; i++) {
        if (taskctl->tasks0[i].flags = 0) {
            task = &taskctl->tasks0[i];
            task->flags = 1;
            task->tss.eflags = 0x00000202;
            task->tss.eax = 0;
            task->tss.ecx = 0;
            task->tss.edx = 0;
            task->tss.ebx = 0;
            task->tss.ebp = 0;
            task->tss.esi = 0;
            task->tss.edi = 0;
            task->tss.es  = 0;
            task->tss.ds  = 0;
            task->tss.fs  = 0;
            task->tss.gs  = 0;
            task->tss.ldtr = 0;
            task->tss.iomap = 0x40000000;
            return task;
        }
    }
    return NULL;
}

void task_run(struct TASK *task) {
    task->flags = 2;
    taskctl->tasks[taskctl->running] = task;
    taskctl->running++;
    return;
}

void task_switch(void) {
    timer_settime(task_timer, 10);
    if (taskctl->running >= 2) {
        taskctl->now++; // 表示当前需要切换的task
        if (taskctl->now == taskctl->running) {
            taskctl->now = 0; // 所有的task都切换了一次后，从头开始
        }
        far_jmp(0, taskctl->tasks0[taskctl->now].sel);
    }
    return;
}