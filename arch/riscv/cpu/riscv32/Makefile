#
#  (c) Copyright 2012-2015 Microsemi Corporation.
#
# SPDX-License-Identifier:	GPL-2.0+
#

extra-y	= start.o

#obj-y   = cache.o cpu.o interrupts.o timer.o
obj-$(if $(filter m2sxxx,$(SOC)),y) += riscv_m2sxxx/
