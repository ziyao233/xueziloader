#	SPDX-License-Identifier: MPL-2.0
#	xueziloader
#	Makefile
#	Copyright (c) 2026 Yao Zi.

ARCH		= $(shell uname -m)
CROSS_COMPILE	=
CC		= $(CROSS_COMPILE)cc
CCAS		= $(CROSS_COMPILE)cc
LD		= $(CROSS_COMPILE)ld
OBJCOPY		= $(CROSS_COMPILE)objcopy

ifeq ($(DEBUG),)
DEBUG_FLAGS	:= -O2
else
DEBUG_FLAGS	:= -O0 -g
endif

ifeq ($(PLATFORM),)
$(error "Please define the targeted PLATFORM")
endif

MYCFLAGS	?= -ffreestanding -fno-stack-protector -fno-stack-check \
		   -fno-pie -static -nostdinc -std=c99 -Wall		\
		   $(DEBUG_FLAGS) $(CFLAGS)

MYCCASFLAGS	?= $(MYCFLAGS) $(CCASFLAGS)
MYLDFLAGS	= $(LDFLAGS) -no-pie

OBJS		= src/start.o src/platform-$(PLATFORM)-early.o

.PHONY: default clean

default: xuezi.bin

xuezi.bin: xuezi.elf
	$(OBJCOPY) -O binary -j .text -j .data $< $@

xuezi.elf: $(OBJS) xuezi.lds
	$(LD) -o $@ $(MYLDFLAGS) $(OBJS) -Txuezi.lds

%.o: %.c
	$(CC) $(MYCFLAGS) -c $< -o $@ -Iinclude -Iinclude/platform-$(PLATFORM)

%.o: %.S
	$(CCAS) $(MYCCASFLAGS) -c $< -o $@ \
		-Iinclude -Iinclude/platform-$(PLATFORM)

clean:
	-rm $(OBJS)
