# include "memory.h"

static volatile unsigned int memset_sub(unsigned int start, unsigned int end);
unsigned int memset(unsigned int start, unsigned int end) {
    char flag486 = 0;
    unsigned int eflag, cr0, i;
    
    // 判断是否为i486及其以上的CPU：i486以下的CPU EFLAG中没有AC位
    eflag = _io_load_eflags();
    eflag |= EFLAGS_AC_BIT;
    _io_restore_eflags(eflag);
    eflag = _io_load_eflags();
    if (eflag & EFLAGS_AC_BIT != 0) {
        flag486 = 1;
    }

    eflag &= ~EFLAGS_AC_BIT;
    _io_restore_eflags(eflag);

    // 如果是i486及其以上的CPU，则先将cache的功能关闭
    if (flag486 != 0) {
        cr0 = _io_load_cr0();
        cr0 |= CR0_CACHE_DISABLE;
        _io_restore_cr0(cr0);
    }

    i = memset_sub(start, end);

    if (flag486 != 0) {
        cr0 = _io_load_cr0();
        cr0 &= ~CR0_CACHE_DISABLE;
        _io_restore_cr0(cr0);
    }

    return i;
}

volatile unsigned int memset_sub(unsigned int start, unsigned int end) {
    unsigned int i, *p, old, pat0 = 0xaa55aa55, pat1 = 0x55aa55aa;
    for (i = start; i<=end; i += 0x1000) {
        p = (unsigned int *) (i + 0xffc);
        old = *p;
        *p = pat0;
        *p ^= 0xffffffff;
        if (*p != pat1) {
not_memory:
            *p = old;
            break;
        }
        *p ^= 0xffffffff;
        if (*p != pat0) {
            goto not_memory;
        }
        *p = old;
    }
    return i;
}
