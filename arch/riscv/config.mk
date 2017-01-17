#
# (C) Copyright 2000-2002
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# Copyright (c) 2016 Microsemi Corporation.
# Padmarao Begari, Microsemi Corporation <padmarao.begari@microsemi.com>
#
# SPDX-License-Identifier:	GPL-2.0+

ifeq ($(CROSS_COMPILE),)
CROSS_COMPILE := riscv32-unknown-linux-gnu-
endif

CONFIG_STANDALONE_LOAD_ADDR = 0x80000000 \
			      -T $(srctree)/examples/standalone/riscv.lds

PLATFORM_CPPFLAGS	+= -D__riscv32__ -ffixed-gp -fpie #-Og -g -march=RV32IM
PLATFORM_RELFLAGS += -fno-strict-aliasing -fno-common 
PLATFORM_RELFLAGS += -gdwarf-2 -m32

# needed for relocation
#LDFLAGS_u-boot += -pie
LDFLAGS_u-boot = --gc-sections