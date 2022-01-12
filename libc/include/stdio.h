#ifndef _STDIO_H_
#define _STDIO_H_

#include <stdarg.h>

#ifndef NULL
#define NULL ((void *)0)
#endif // NULL

int sprintf(char *s, const char *format, ...);
int vsprintf(char *s, const char *format, va_list args);

#endif
