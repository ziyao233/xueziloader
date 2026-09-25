// SPDX-License-Identifier: MPL-2.0
/*
 *	xueziloader
 *	include/barrier.h
 *	Copyright (C) 2026 Yao Zi <me@ziyao.cc>
 */

#ifndef _BARRIER_H_
#define _BARRIER_H_

#define barrier()	asm volatile ("" : : : "memory")
#define nop()		asm ("nop")

/* Let's be restrictive here. In bootloader, a full dbar won't hurt in anyway */
#define mb()		asm ("dbar 0")
#define wmb()		mb()
#define rmb()		mb()

#endif // _BARRIER_H_
