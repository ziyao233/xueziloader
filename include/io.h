// SPDX-License-Identifier: MPL-2.0
/*
 *	xueziloader
 *	include/io.h
 *	Copyright (C) 2026 Yao Zi <me@ziyao.cc>
 */

#ifndef _IO_H_
#define _IO_H_

#include <barrier.h>
#include <stdint.h>

#define __read(name, t) \
static inline t name(volatile void *mem)				\
{									\
	t value;							\
									\
	barrier();							\
	value = *(volatile t *)mem;					\
	rmb();								\
									\
	return value;							\
}

__read(readb, uint8_t)
__read(readw, uint16_t)
__read(readl, uint32_t)
__read(readq, uint64_t)

#undef __read

#define __write(name, t) \
static inline void name(t value, volatile void *mem)			\
{									\
	wmb();								\
	*(volatile t *)mem = value;					\
	barrier();							\
}

__write(writeb, uint8_t)
__write(writew, uint16_t)
__write(writel, uint32_t)
__write(writeq, uint64_t)

#undef __write

#endif // _IO_H_
