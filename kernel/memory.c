#include "memory.h"
#include "screen.h"
#include "stdio.h"
#include "ports.h"

static unsigned int memtest_sub(unsigned int start, unsigned int end);

unsigned int memtest(unsigned int start, unsigned int end) {
    char flg486 = 0;
    unsigned int eflg, cr0;

    eflg = _io_load_eflags();
    eflg |= EFLAGS_AC_BIT; // AC-bit = 1

    _io_restore_eflags(eflg);
    eflg = _io_load_eflags();
    if ((eflg & EFLAGS_AC_BIT)) {
        // 如果是i386，即使设定AC=1，也会自动变回到0
        flg486 = 1;
    }

    eflg &= ~EFLAGS_AC_BIT; // AC-bit = 0
    _io_restore_eflags(eflg);

    if (flg486) {
        cr0 = _io_load_cr0();
        cr0 |= CR0_CACHE_DISABLE; // 禁止缓存
        _io_restore_cr0(cr0);
    }

    unsigned int ret = memtest_sub(start, end);

    if (flg486) {
        cr0 = _io_load_cr0();
        cr0 &= ~CR0_CACHE_DISABLE; // 允许缓存
        _io_restore_cr0(cr0);
    }

    return ret;
}

static unsigned int memtest_sub(unsigned int start, unsigned int end) {
    unsigned int i, *p, old, pat0 = 0xaa55aa55, pat1 = 0x55aa55aa;

    for (i = start; i <= end; i += 0x1000) {
        p = (unsigned int *)(i + 0xffc);

        old = *p;         // 先记住修改前的值
        *p = pat0;        // 试写
        *p ^= 0xffffffff; // 反转

        if (*p != pat1) {
            // 检查反转结果
not_memory:
            *p = old;
            break;
        }

        *p ^= 0xffffffff; // 再次反转
        if (*p != pat0) {
            // 检查值是否恢复
            goto not_memory;
        }
        *p = old;
    }

    return i;
}

void memman_init(struct MEMMAN *man) {
    man->frees = 0; // 可用信息数目
    man->maxfrees = 0; // frees的最大值， 用于调试？
    man->lostsize = 0; // 释放失败的内存的大小总和
    man->losts = 0; // 释放失败次数
    return;
}

unsigned int memman_total(struct MEMMAN *man) {
    unsigned int i, t = 0;
    for (i = 0; i < man->frees; i++) {
        t += man->free[i].size;
    }
    return t;
}

unsigned int memman_alloc(struct MEMMAN *man, unsigned int size) {
    unsigned int i, a;
    for (i = 0; i < man->frees; i++) {
        if (man->free[i].size >= size) {
            a = man->free[i].addr;
            man->free[i].addr += size;
            man->free[i].size -= size;
            if (man->free[i].size == 0) {
                man->frees--;
                for (; i<man->frees; i++) {
                    man->free[i] = man->free[i+1];
                }
            }
            return a;
        }
    }
    return 0; // 表示没有可用空间
}

int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size) {
  int i, j;

  for (i = 0; i < man->frees; i++) {
    if (man->free[i].addr > addr) {
      break;
    }
  }

  // free[i - 1].addr < addr < free[i].addr
  if (i > 0) {
    // 前面有可用内存
    if (man->free[i - 1].addr + man->free[i - 1].size == addr) {
      // 可以与前面的可用内存归纳到一起
      man->free[i - 1].size += size;
      if (i < man->frees) {
        // 后面也是
        if (addr + size == man->free[i].addr) {
          // 也可以与后面的可用内存归纳到一起
          man->free[i - 1].size += man->free[i].size;
          // man->free[i]删除
          // free[i]变成0后归纳到前面去
          man->frees--;
          for (; i < man->frees; i++) {
            man->free[i] = man->free[i + 1]; // 结构体赋值
          }
        }
      }
    }
  }

  // 不能与前面的可用空间归纳到一起
  if (i < man->frees) {
    // 后面还有
    if (addr + size == man->free[i].addr) {
      man->free[i].addr = addr;
      man->free[i].size += size;
      return 0; // 成功完成
    }
  }

  // 既不能与前面归纳到一起，也不能与后面归纳到一起
  if (man->frees < MEMMAN_FREES) {
    // free[i]之后的，向后移动，腾出一点可用空间
    for (j = man->frees; j > i; j--) {
      man->free[j] = man->free[j - 1];
    }

    man->frees++;

    if (man->maxfrees < man->frees) {
      man->maxfrees = man->frees; // 更新最大值
    }

    man->free[i].addr = addr;
    man->free[i].size = size;
    return 0; // 成功完成
  }

  // 不能往后移动
  man->losts++;
  man->lostsize += size;
  return -1;
}

unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size) {
  size = (size + 0xfff) & 0xfffff000;
  return memman_alloc(man, size);
}


int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size) {
    size = (size + 0xfff) & 0xfffff000;
    return memman_free(man, addr, size);
}