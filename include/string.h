// SPDX-License-Identifier: MPL-2.0
/*
 *	xueziloader
 *	include/string.h
 *	Copyright (C) 2026 Yao Zi <me@ziyao.cc>
 */

#ifndef _STRING_H_
#define _STRING_H_

#include <stddef.h>

size_t strlen(const char *p);
char *strcpy(char *dst, const char *src);
void *memcpy(void *dst, const void *src, size_t n);
void *memmove(void *dst, const void *src, size_t n);
void *memset(void *mem, int c, size_t n);
int memcmp(const void *a, const void *b, size_t len);

#endif // _STRING_H_
