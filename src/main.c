// SPDX-License-Identifier: MPL-2.0
/*
 *	xueziloader
 *	src/main.c
 *	Copyright (C) 2026 Yao Zi <me@ziyao.cc>
 */

void rawputc(char c);
void
putc(char c)
{
	if (c == '\n')
		rawputc('\r');
	rawputc(c);
}

int
puts(const char *s)
{
	while (*s)
		putc(*s++);
	putc('\n');

	return 0;
}

void
main(void)
{
	puts("\nHello xueziloader");

	while (1);
}
