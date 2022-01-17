#include "ports.h"

#ifndef _MEMORY_H_
#define _MEMORY_H_

#define EFLAGS_AC_BIT     0x00040000
#define CR0_CACHE_DISABLE 0x60000000

unsigned int memset(unsigned int start, unsigned int end);

#endif
