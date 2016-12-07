/*
 * Copyright (C) 2016 Microsemi Corporation
 *
 * Modified to support C structur SoC access by
 * Padmarao Begari <padmarao.begari@microsemi.com>
 *
 * SPDX-License-Identifier: GPL-2.0+
 */
#include <common.h>
#include <dm.h>
#include <errno.h>
#include <watchdog.h>
#include <serial.h>
#include <linux/compiler.h>

#include <asm/io.h>
#ifdef CONFIG_DM_SERIAL
#include <asm/arch/msc_serial.h>
#endif


DECLARE_GLOBAL_DATA_PTR;

/* CoreUART register footprint */
typedef struct msc_coreuart {
    u32 txdata;
    u32 rxdata;
    u32 ctrl1;
    u32 ctrl2;
    u32 status;
    u32 ctrl3;
} msc_coreuart_t;

#define DATA_8_BITS     0x01u
#define NO_PARITY       0x00u

#define BAUDVALUE_LSB       (0x00FF) 
#define BAUDVALUE_MSB       (0xFF00) 
#define BAUDVALUE_SHIFT     (5) 

#define STATUS_RXFULL_MASK     0x02u
#define STATUS_TXRDY_MASK     0x01u


static void msc_serial_setbrg_internal(msc_coreuart_t *usart, int id,
      int baudrate)
{
    u32 baud_value;
    u8 baud_value_low;
    u8 baud_value_high;

   /*
    * BAUD_VALUE = (CLOCK / (16 * BAUD_RATE)) - 1
    */
    baud_value = (CONFIG_SYS_COREUART_CLK / (16 * baudrate)) -1; 
    baud_value_low = baud_value & BAUDVALUE_LSB;
    baud_value_high = (baud_value & BAUDVALUE_MSB) >> BAUDVALUE_SHIFT;
    
    writeb(baud_value_low, &usart->ctrl1);
    writeb(baud_value_high, &usart->ctrl2);
}

static void msc_serial_activate(msc_coreuart_t *usart)
{
    u32 value;
    u32 rx_full;
    u8 rx_byte;
    
    value = readb(&usart->ctrl2) | (DATA_8_BITS | NO_PARITY);
    writeb(value, &usart->ctrl2);
    
    rx_full = readb(&usart->status) & STATUS_RXFULL_MASK;
    
    while (rx_full)
    {
            rx_byte = readb(&usart->rxdata);
            rx_full = readb(&usart->status) & STATUS_RXFULL_MASK;
    }
}

#ifndef CONFIG_DM_SERIAL
static void msc_serial_setbrg(void)
{
    msc_serial_setbrg_internal((msc_coreuart_t *)CONFIG_USART_BASE,
            CONFIG_USART_ID, gd->baudrate);
}

static int msc_serial_init(void)
{
    msc_coreuart_t *usart = (msc_coreuart_t *)CONFIG_USART_BASE;

    msc_serial_setbrg();
    msc_serial_activate(usart);

    return 0;
}

static void msc_serial_putc(char c)
{
    msc_coreuart_t *usart = (msc_coreuart_t *)CONFIG_USART_BASE;

    if (c == '\n')
        serial_putc('\r');

    while (!(readb(&usart->status) & STATUS_TXRDY_MASK));
    usart->txdata = c;
}

static int msc_serial_getc(void)
{
    msc_coreuart_t *usart = (msc_coreuart_t *)CONFIG_USART_BASE;

    while (!(readb(&usart->status) & STATUS_RXFULL_MASK))
    {
        /*  WATCHDOG_RESET(); */
    }
    return readb(&usart->rxdata);
}

static int msc_serial_tstc(void)
{
    msc_coreuart_t *usart = (msc_coreuart_t *)CONFIG_USART_BASE;
    return (readb(&usart->status) & STATUS_RXFULL_MASK) != 0;
}

static struct serial_device msc_serial_drv = {
    .name = "msc_serial",
    .start = msc_serial_init,
    .stop = NULL,
    .setbrg = msc_serial_setbrg,
    .putc = msc_serial_putc,
    .puts = default_serial_puts,
    .getc = msc_serial_getc,
    .tstc = msc_serial_tstc,
};

void msc_serial_initialize(void)
{
    serial_register(&msc_serial_drv);
}

__weak struct serial_device *default_serial_console(void)
{
    return &msc_serial_drv;
}
#endif
