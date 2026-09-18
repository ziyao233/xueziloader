// SPDX-License-Identifer: MPL-2.0
/*
 *	xueziloader
 *	include/stdarg.h
 *	Copyright (C) 2026 Yao Zi <me@ziyao.cc>
 */

#ifndef _STDARG_H_
#define _STDARG_H_

typedef __builtin_va_list	va_list;
#define va_start(v, l)		__builtin_va_start(v, l)
#define va_end(v)		__builtin_va_end(v)
#define va_arg(v, t)		__builtin_va_arg(v, t)
#define va_copy(d, s)		__builtin_va_copy(d, s)

#endif // _STDARG_H_
