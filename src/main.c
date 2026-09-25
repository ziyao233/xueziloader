// SPDX-License-Identifier: MPL-2.0
/*
 *	xueziloader
 *	src/main.c
 *	Copyright (C) 2026 Yao Zi <me@ziyao.cc>
 */

#include <platform.h>
#include <stdio.h>

void
main(void)
{
	puts("\nHello xueziloader");

	platform_late_init();

	while (1);
}
