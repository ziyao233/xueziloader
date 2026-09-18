// SPDX-License-Identifier: MPL-2.0
/*
 *	xueziloader
 *	src/string.c
 *	Copyright (c) 2024-2026 Yao Zi <me@ziyao.cc>
 */

#include <stddef.h>
#include <string.h>

size_t
strlen(const char *p)
{
	size_t len = 0;
	while (*p++)
		len++;
	return len;
}

char *
strcpy(char *dst, const char *src)
{
	char *org = dst;
	while (*src)
		*(dst++) = *(src++);
	*dst = 0;
	return org;
}

void *
memcpy(void *dst, const void *src, size_t n)
{
	const char *pSrc = src;
	char *pDst = dst;
	while (n--)
		*(pDst++) = *(pSrc++);
	return dst;
}

void *
memmove(void *dst, const void *src, size_t n)
{
	return memcpy(dst, src, n);
}

void *
memset(void *mem, int c, size_t n)
{
	char *p = mem;
	while (n--)
		*(p++) = c;
	return mem;
}

int
memcmp(const void *a, const void *b, size_t len)
{
	const char *c = a, *d = b;
	while (len--) {
		if (*c != *d)
			return *c - *d;

		c++;
		d++;
		len--;
	}

	return 0;
}
