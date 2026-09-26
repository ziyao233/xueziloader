// SPDX-License-Identifier: MPL-2.0
/*
 *	xueziloader
 *	src/lowlevel.S
 *	Copyright (C) 2026 Yao Zi <me@ziyao.cc>
 */

	.global		exception_handler
	.p2align	12
exception_handler:
	nop
	b		exception_handler
