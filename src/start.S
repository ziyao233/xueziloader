// SPDX-License-Identifier: MPL-2.0
/*
 *	xueziloader
 *	src/start.S
 *	Copyright (C) 2026 Yao Zi.
 */

	.section .text.init
	.global		_start

_start:
	b		_start

	.data

hello:	.asciz		"hello world\r\n"
