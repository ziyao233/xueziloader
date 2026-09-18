// SPDX-License-Identifier: MPL-2.0
/*
 *	xueziloader
 *	src/stdio.c
 *	Copyright (c) 2024-2026 Yao Zi <me@ziyao.cc>
 */

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static int
itoa_n(char *out, unsigned long int n, int base)
{
	static const char digits[] = "0123456789abcdef";

	if (n == 0) {
		out[0] = '0';
		return 1;
	}

	int len = 0;
	while (n) {
		out[len] = digits[n % base];
		n /= base;
		len++;
	}

	char tmp;
	for (int i = 0; i < len / 2; i++) {
		tmp = out[len - i - 1];
		out[len - i - 1] = out[i];
		out[i] = tmp;
	}

	return len;
}

void
vsprintf(char *p, const char *format, va_list va)
{
	bool longValue;
	int base = 10;
	long int value;
	while (*format)
	switch (*format) {
	case '%':
		format++;
		if (*format == '%') {
			*(p++) = *(format++);
			break;
		} else if (*format == 's') {
			format++;
			const char *src = va_arg(va, const char *);
			strcpy(p, src);
			p += strlen(src);
			break;
		} else if (*format == 'c') {
			format++;
			*(p++) = va_arg(va, int);
			break;
		}

		switch (*format) {
		case 'l':
			format++;
			// fallthrough
		case 'p':
			value = va_arg(va, long int);
			longValue = 1;
			break;
		default:
			value = va_arg(va, int);
			longValue = 0;
		}

		switch (*format) {
		case 'd':
			if (value < 0) {
				*(p++) = '-';
				value = -value;
			}
			// fallthrough
		case 'u':
			base = 10;
			break;
		case 'p':
			p[0] = '0';
			p[1] = 'x';
			p += 2;
			// fallthrough
		case 'x':
			base = 16;
			break;
		}
		format++;

		p += itoa_n(p, longValue ? value : (value & 0xffffffff), base);
		break;
	default:
		*(p++) = *(format++);
		break;
	}

	*p = '\0';
}

void rawputc(char c);

void
putc(char c)
{
	if (c == '\n')
		rawputc('\r');
	rawputc(c);
}

void
puts(const char *s)
{
	while (*s)
		putc(*s++);
	putc('\n');
}

void
printf(const char *fmt, ...)
{
	char buf[256] = { 0 };
	va_list va;

	va_start(va, fmt);
	vsprintf(buf, fmt, va);
	va_end(va);

	const char *p = buf;
	while (*p)
		putc(*p++);
}
