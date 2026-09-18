// SPDX-License-Identifier: MPL-2.0
/*
 *	xueziloader
 *	src/stdio.c
 *	Copyright (C) 2026 Yao Zi <me@ziyao.cc>
 */

#ifndef _STDIO_H_
#define _STDIO_H_

#include <stdarg.h>

void vsprintf(char *p, const char *fmt, va_list va);
void putc(char c);
void puts(const char *s);
void printf(const char *fmt, ...);

#endif // _STDIO_H_
