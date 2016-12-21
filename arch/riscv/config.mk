#
# (C) Copyright 2000-2002
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# Copyright (C) 2016 Microsemi Corporation.
# Padmarao Begari, Microsemi Corporation <padmarao.begari@microsemi.com>
#
# SPDX-License-Identifier:	GPL-2.0+

ifeq ($(CROSS_COMPILE),)
CROSS_COMPILE := riscv32-unknown-linux-gnu-
endif

CONFIG_STANDALONE_LOAD_ADDR = 0x80100000 \
			      -T $(srctree)/examples/standalone/riscv.lds

#LDFLAGS_FINAL += --gc-sections --relax
#PLATFORM_RELFLAGS += -ffunction-sections -fdata-sections -fno-common -ffixed-gp
PLATFORM_CPPFLAGS	+= -D__riscv32__ -ffixed-gp -Og -g -march=RV32IM 
#PLATFORM_RELFLAGS += -fno-strict-aliasing -fno-common -remap
#PLATFORM_CPPFLAGS	+= -D__riscv32__ -ffixed-gp -gdwarf-2 -m32 -march=RV32IM -fpie

# needed for relocation
#LDFLAGS_u-boot += -pie