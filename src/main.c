// SPDX-License-Identifier: MPL-2.0
/*
 *	xueziloader
 *	src/main.c
 *	Copyright (C) 2026 Yao Zi <me@ziyao.cc>
 */

#include <addrspace.h>
#include <barrier.h>
#include <csr.h>
#include <platform.h>
#include <stdio.h>

void exception_handler(void);

static void
setup_exception_handler(void)
{
	csr_write(exception_handler, LOONGARCH_CSR_EENTRY);
	csr_write(TO_PHYS((uint64_t)exception_handler),
		  LOONGARCH_CSR_TLBRENTRY);
}

void
main(void)
{
	puts("\nHello xueziloader");

	setup_exception_handler();

	platform_late_init();

	while (1);
}
