/*
 * Microsemi CoreSPI interface (SPI mode)
 *
 * Copyright (c ) 2016  Microsemi corporation
 * Written-by: Padmarao Begari <padmarao.begari@microsemi.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <linux/compat.h>
#include <asm/io.h>
#include <malloc.h>
#include <spi.h>

#define CTRL1_ENABLE_MASK           0x01u
#define CTRL1_MASTER_MASK           0x02u

#define CMD_RXFIFORST_MASK          0x01u
#define CMD_TXFIFORST_MASK          0x02u

#define STATUS_RXOVFLOW_SHIFT       0x04
/* For clearing all active interrupts */
#define SPI_ALL_INTS                0xFF
 
/* CoreSPI registers */
struct corespi_regs {
    u32 crtl1;
    u32 intclear;
    u32 rxdata;
    u32 txdata;
    u32 intmask;
    u32 intraw;
    u32 crtl2;
    u32 command;
    u32 stat;
    u32 ssel;
    u32 txdata_last;
};

/* corespi slave */
struct corespi_slave {
    struct spi_slave slave;
    struct corespi_regs *regs;
};
static u8 backup_data[128];
static u8 backup_len;

static void recover_from_rx_overflow(struct corespi_slave *as);
static void wait_ready(struct spi_slave *slave);
static void corespi_transfer_block
(
	struct spi_slave *slave,
    const u8 * cmd_buffer,
    u16 cmd_byte_size,
    u8 * rx_buffer,
    u16 rx_byte_size
);
static inline struct corespi_slave *to_corespi_slave(
    struct spi_slave *slave)
{
    return container_of(slave, struct corespi_slave, slave);
}

/* spi_init is called during boot when CONFIG_CMD_SPI is defined */
void spi_init(void)
{
   /*
    * configuration will be done in spi_setup_slave()
    */
}
/* the following is called in sequence by do_spi_xfer() */

struct spi_slave *spi_setup_slave(uint bus, uint cs, uint max_hz, uint mode)
{
    struct corespi_slave *cslave;
    u8 command;
    u8 din[1];
    /* we only set up CoreSPI for now, so ignore bus */

    if (mode & SPI_3WIRE) {
        error("3-wire mode not supported");
        return NULL;
    }

    if (mode & SPI_SLAVE) {
        error("slave mode not supported\n");
        return NULL;
    }

    if (mode & SPI_PREAMBLE) {
        error("preamble byte skipping not supported\n");
        return NULL;
    }

    cslave = spi_alloc_slave(struct corespi_slave, bus, cs);
    if (!cslave) {
        printf("SPI_error: Fail to allocate corespi_slave\n");
        return NULL;
    }

    cslave->regs = (struct corespi_regs *)CORESPI_BASE_ADDRESS;

    /* Ensure all slaves are deselected */
    writeb(0, &cslave->regs->ssel);
    /* Flush the receive and transmit FIFOs*/
    writeb(CMD_RXFIFORST_MASK | CMD_TXFIFORST_MASK, &cslave->regs->command);
    /* Clear all interrupts */
    writeb(SPI_ALL_INTS, &cslave->regs->intclear);
    /* Ensure RXAVAIL, TXRFM, SSEND and CMDINT are disabled */
    writeb(0, &cslave->regs->crtl2);
    /* Enable the CoreSPI in master mode with TXUNDERRUN, RXOVFLOW and TXDONE interrupts disabled */
    writeb(CTRL1_ENABLE_MASK | CTRL1_MASTER_MASK, &cslave->regs->crtl1);
#ifdef CONFIG_SPI_FLASH_STMICRO
    writeb(0x1, &cslave->regs->ssel);
    command = 0x66;
    corespi_transfer_block(&cslave->slave, &command, 1, din, 0);
    command = 0x99;
    corespi_transfer_block(&cslave->slave, &command, 1, din, 0);
	wait_ready(&cslave->slave);

#endif
    return &cslave->slave;
}

void spi_free_slave(struct spi_slave *slave)
{
    struct corespi_slave *cslave = to_corespi_slave(slave);

    debug("(corespi_free_slave: 0x%08x\n", (u32)cslave);
    free(cslave);
}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen,
    const void *dout, void *din, unsigned long flags)
{
 /*   struct corespi_slave *cslave = to_corespi_slave(slave);*/
    unsigned int bytelen = bitlen >> 3;
    const u8 *txp = dout;
    u8 *rxp = din;
    
    if (bitlen == 0)
        /* Finish any previously submitted transfers */
        goto out;

    /*
     * TODO: The controller can do non-multiple-of-8 bit
     * transfers, but this driver currently doesn't support it.
     *
     * It's also not clear how such transfers are supposed to be
     * represented as a stream of bytes...this is a limitation of
     * the current SPI interface.
     */
    if (bitlen % 8) {
        /* Errors always terminate an ongoing transfer */
        flags |= SPI_XFER_END;
        goto out;
    }

    if (flags & SPI_XFER_BEGIN)
    {
        spi_cs_activate(slave);
    }
    if(din == NULL)
    {
    	/*  write */
    	for(u8 i = 0; i < bytelen; i++)
    		backup_data[i] = txp[i];
        backup_len = bytelen;
    }
    else if(dout == NULL)
    {
    	/* read */
        corespi_transfer_block(slave, backup_data, backup_len, rxp, bytelen);
    }

    
out:
    if (flags & SPI_XFER_END)
    {
        spi_cs_deactivate(slave);
    }
    return 0;
}
int spi_claim_bus(struct spi_slave *slave)
{
    struct corespi_slave *as = to_corespi_slave(slave);
    /* Enable the CoreSPI */
    writeb(CTRL1_ENABLE_MASK | CTRL1_MASTER_MASK, &as->regs->crtl1);

    return 0;
}
void spi_release_bus(struct spi_slave *slave)
{
    struct corespi_slave *as = to_corespi_slave(slave);
    /* Disable the CoreSPI */
    writeb(0, &as->regs->crtl1);

}
int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
    return 0;
}

void spi_cs_activate(struct spi_slave *slave)
{
    struct corespi_slave *as = to_corespi_slave(slave);
    recover_from_rx_overflow(as);
    /* Set the correct slave select bit */
    writeb((1 << CORESPI_SLAVE_SELECT), &as->regs->ssel);
}

void spi_cs_deactivate(struct spi_slave *slave)
{
    struct corespi_slave *as = to_corespi_slave(slave);
    recover_from_rx_overflow(as);
    /* Clear the correct slave select bit */
    writeb(0, &as->regs->ssel);
}
static void recover_from_rx_overflow(struct corespi_slave *as)
{
    u8 temp;
    temp = readb(as->regs->stat);
     /* Recover from receiver overflow because of previous slave */
    if(((temp >> STATUS_RXOVFLOW_SHIFT) & 0x1) == 0x1)
    {
         /* Disable the CoreSPI */
         writeb(0, &as->regs->crtl1);
        /* Flush the receive and transmit FIFOs*/
        writeb(CMD_RXFIFORST_MASK | CMD_TXFIFORST_MASK, &as->regs->command);
        /* Clear all interrupts */
        writeb(SPI_ALL_INTS, &as->regs->intclear);
        /* Enable the CoreSPI */
        writeb(CTRL1_ENABLE_MASK | CTRL1_MASTER_MASK, &as->regs->crtl1);
    }
}
static void wait_ready(struct spi_slave *slave)
{
    u8 ready_bit;
    u8 command = 0x05; /* READ_STATUS */

    do {
    	corespi_transfer_block(slave, &command, 1, &ready_bit, 1);
        ready_bit = ready_bit & 0x01;
    } while(ready_bit & 0x01);
}
static void corespi_transfer_block
(
	struct spi_slave *slave,
    const u8 * cmd_buffer,
    u16 cmd_byte_size,
    u8 * rx_buffer,
    u16 rx_byte_size
)
{
    struct corespi_slave *cspi = to_corespi_slave(slave);
    u32 transfer_size = 0;   /* Total number of bytes to  transfer. */
    u16 transfer_idx = 0;    /* Number of bytes transferred so far */
    u16 tx_idx = 0;          /* Number of valid data bytes sent */
    u16 rx_idx = 0;          /* Number of valid response bytes received */
    u16 transit = 0;         /* Number of bytes "in flight" to avoid FIFO errors */
    u8 p_reg;

    /*
     * tansfer_size is one less than the real amount as we have to write
     * the last frame separately to trigger the slave deselect in case
     * the SPS option is in place.
     */
    transfer_size = ((u32)cmd_byte_size + (u32)rx_byte_size) - 1;
            /* Flush the receive and transmit FIFOs */
    writeb(CMD_RXFIFORST_MASK | CMD_TXFIFORST_MASK, &cspi->regs->command);
    /* Recover from receiver overflow because of previous slave */
    recover_from_rx_overflow(cspi);

	p_reg = readb(&cspi->regs->crtl1);
	p_reg &= ~(0x1);
	/* Disable the Core SPI for a little bit, while we load the TX FIFO */
	writeb(p_reg, &cspi->regs->crtl1);

	while((tx_idx < transfer_size) && (tx_idx < 32))
	{
		if( tx_idx < cmd_byte_size )
	    {
	        	/* Push out valid data */
			writel((u32)cmd_buffer[tx_idx], &cspi->regs->txdata);
	    }
	    else
	    {
			/* Push out 0s to get data back from slave */
	    	writel(0x0, &cspi->regs->txdata);
	    }
	    ++transit;
	    ++tx_idx;
	}

	/* If room left to put last frame in before the off, then do it */
	if((tx_idx == transfer_size) && (tx_idx < 32))
	{
		if( tx_idx < cmd_byte_size )
		{
	       	/* Push out valid data, not expecting any reply this time */
			writel((u32)cmd_buffer[tx_idx], &cspi->regs->txdata_last);
	    }
	    else
	    {
			/* Push out last 0 to get data back from slave */
	       	writel(0x0, &cspi->regs->txdata_last);
	    }
		++transit;
		++tx_idx;
	}

	/* FIFO is all loaded up so enable Core SPI to start transfer */
	p_reg = readb(&cspi->regs->crtl1);
    p_reg |= 0x01;
    writeb(p_reg, &cspi->regs->crtl1);

    /* Perform the remainder of the transfer by sending a byte every time a byte
     * has been received. This should ensure that no Rx overflow can happen in
     * case of an interrupt occurring during this function.
     *
     * We break the transfer down into stages to minimise the processing in
     * each loop as the SPI interface is very demanding at higher clock rates.
     * This works well with FIFOs but might be less efficient if there is only
     * a single frame buffer.
     *
     * First stage transfers remaining command bytes (if any).
     * At this stage anything in the RX FIFO can be discarded as it is
     * not part of a valid response.
     */

    while( tx_idx < cmd_byte_size )
	{
    	if( transit < 32 )
		{
		      /* Send another byte. */
		   	if( tx_idx == transfer_size ) /* Last frame is special... */
		   	{
		   		writel((u32)cmd_buffer[tx_idx], &cspi->regs->txdata_last);
		   	}
		   	else
		   	{
		   		writel((u32)cmd_buffer[tx_idx], &cspi->regs->txdata);
		   	}
		   	++tx_idx;
		    ++transit;
		}
    	if(!((readb(&cspi->regs->stat) >> 0x2) & 0x1))
		{
		     /* Read and discard. */
    		readl(&cspi->regs->rxdata);
		   	++transfer_idx;
		    --transit;
		}
    }
    /*
     * Now, we are writing dummy bytes to push through the response from
     * the slave but we still have to keep discarding any read data that
     * corresponds with one of our command bytes.
     */
    while( transfer_idx < cmd_byte_size )
	{
    	if( transit < 32 )
		{
    		if( tx_idx < transfer_size )
		    {
    			writel(0x0, &cspi->regs->txdata);
				++tx_idx;
				++transit;
		    }
		}
    	if(!((readb(&cspi->regs->stat) >> 0x2) & 0x1))
		{
		    /* Read and discard. */
    		readl(&cspi->regs->rxdata);
		   	++transfer_idx;
		    --transit;
		}
    }
    /*
     * Now we are now only sending dummy data to push through the
     * valid response data which we store in the response buffer.
     */
    while( tx_idx < transfer_size )
	{
    	if( transit < 32 )
		{
    		writel(0x0, &cspi->regs->txdata);
			++tx_idx;
			++transit;
		}
    	if(!((readb(&cspi->regs->stat) >> 0x2) & 0x1))
		{
		    /* Process received byte. */
			rx_buffer[rx_idx] = (u8)readl(&cspi->regs->rxdata);
			++rx_idx;
		    ++transfer_idx;
		    --transit;
		}
    }
    /* If we still need to send the last frame */
	while( tx_idx == transfer_size )
	{
		if( transit < 32 )
		{
			writel(0x0, &cspi->regs->txdata_last);
			++tx_idx;
			++transit;
		}
		if(!((readb(&cspi->regs->stat) >> 0x2) & 0x1))
		{
		    /* Process received byte. */
			rx_buffer[rx_idx] = (u8)readl(&cspi->regs->rxdata);
			++rx_idx;
		    ++transfer_idx;
		    --transit;
		}
	}

    /*
     * Finally, we are now finished sending data and are only reading
     * valid response data which we store in the response buffer.
     */
    while( transfer_idx <= transfer_size )
	{
		if(!((readb(&cspi->regs->stat) >> 0x2) & 0x1))
	    {
	        /* Process received byte. */
			rx_buffer[rx_idx] = (u8)readl(&cspi->regs->rxdata);
			++rx_idx;
	        ++transfer_idx;
	    }
    }

}

