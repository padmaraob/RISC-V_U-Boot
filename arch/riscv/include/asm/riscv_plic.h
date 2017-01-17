/*
 * Copyright (c) 2016 Microsemi Corporation.
 * Copyright (c) 2016 Padmarao Begari <Padmarao.Begari@microsemi.com>
 */
#ifndef RISCV_PLIC_H
#define RISCV_PLIC_H

#ifdef CONFIG_USE_IRQ

#include "encoding.h"

#define PLIC_NUM_SOURCES 31
#define PLIC_NUM_PRIORITIES 0

/*==============================================================================
 * Interrupt numbers:
 */
typedef enum
{
    NoInterrupt_IRQn = 0,
    External_1_IRQn  = 1,
    External_2_IRQn  = 2,
    External_3_IRQn  = 3, 
    External_4_IRQn  = 4,
    External_5_IRQn  = 5,
    External_6_IRQn  = 6,
    External_7_IRQn  = 7,
    External_8_IRQn  = 8,
    External_9_IRQn  = 9,
    External_10_IRQn = 10,
    External_11_IRQn = 11,
    External_12_IRQn = 12,
    External_13_IRQn = 13,
    External_14_IRQn = 14,
    External_15_IRQn = 15,
    External_16_IRQn = 16,
    External_17_IRQn = 17,
    External_18_IRQn = 18,
    External_19_IRQn = 19,
    External_20_IRQn = 20,
    External_21_IRQn = 21,
    External_22_IRQn = 22,
    External_23_IRQn = 23,
    External_24_IRQn = 24,
    External_25_IRQn = 25,
    External_26_IRQn = 26,
    External_27_IRQn = 27,
    External_28_IRQn = 28,
    External_29_IRQn = 29,
    External_30_IRQn = 30,
    External_31_IRQn = 31
} IRQn_Type;

/*==============================================================================
 * PLIC: Platform Level Interrupt Controller
 */
#define PLIC_BASE_ADDR 0x40000000

typedef struct
{
	volatile uint priority_threshold;
    volatile uint claim_complete;
    volatile uint reserved[1022];
} IRQ_Target_Type;

typedef struct
{
    volatile uint enables[32];
} Target_Enables_Type;

typedef struct
{
    volatile uint reserved;
    
    /*-------------------- Source Priority --------------------*/
    volatile uint source_priority[1023];
    
    /*-------------------- Pending array --------------------*/
    volatile const uint pending_array[32];
    volatile uint reserved1[992];
    
    /*-------------------- Target enables --------------------*/
    volatile Target_Enables_Type target_enables[15872];
    volatile uint reserved2[32768];
    
    /*-------------------- Target enables --------------------*/
    IRQ_Target_Type target[15872];
    
} plic_type;

#define plic    ((plic_type *)PLIC_BASE_ADDR)

/*==============================================================================
 * PRCI: Power, Reset, Clock, Interrupt
 */
#define PRCI_BASE   0x44000000

typedef struct
{
    volatile unsigned int msip[4095];
    volatile unsigned long long mtimecmp[4095];
    volatile const unsigned long long mtime;
} prci_type;

#define prci    ((prci_type *)PRCI_BASE)

/*==============================================================================
 * Move the following to appropriate header.
 */
static inline void plic_init(void)
{
	uint inc;
	unsigned long hart_id = read_csr(mhartid);

	/* Disable all interrupts for the current hart. */
	for(inc = 0; inc < ((PLIC_NUM_SOURCES + 32) / 32); ++inc)
	{
		plic->target_enables[hart_id].enables[inc] = 0;
	}

	/* Set priorities to zero. */
	/* Should this really be done??? Calling PLIC_init() on one hart will cause
	 * the priorities previously set by other harts to be messed up. */
	for(inc = 0; inc < PLIC_NUM_SOURCES; ++inc)
	{
        plic->source_priority[inc] = 0;
    }

	/* Set the threshold to zero. */
	plic->target[hart_id].priority_threshold = 0;
}

static inline void plic_enable_irq(IRQn_Type IRQn)
{
    unsigned long hart_id = read_csr(mhartid);
    uint current = plic->target_enables[hart_id].enables[IRQn / 32];
    current |= (uint)1 << (IRQn % 32);
    plic->target_enables[hart_id].enables[IRQn / 32] = current;
}


static inline void plic_disable_irq(IRQn_Type IRQn)
{
    unsigned long hart_id = read_csr(mhartid);
    uint current = plic->target_enables[hart_id].enables[IRQn / 32];
    current &= ~((uint)1 << (IRQn % 32));
    plic->target_enables[hart_id].enables[IRQn / 32] = current;
}


static inline void plic_set_priority(IRQn_Type IRQn, uint priority)
{
    plic->source_priority[IRQn] = priority;
}


static inline uint plic_get_priority(IRQn_Type IRQn)
{
    return plic->source_priority[IRQn];
}


static inline uint plic_claim_irq(void)
{
	unsigned long hart_id = read_csr(mhartid);

	return plic->target[hart_id].claim_complete;
}


static inline void plic_complete_irq(uint source)
{
	unsigned long hart_id = read_csr(mhartid);

	plic->target[hart_id].claim_complete = source;
}

#endif /* CONFIG_USE_IRQ */
#endif  /* RISCV_PLIC_H */

