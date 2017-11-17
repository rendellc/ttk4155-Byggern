/*!@file
*  implement functions for controlling MCP2515 Can controller
*/

#include "global_declarations.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#include "mcp2515.h"
#include "spi_driver.h"
#include "uart.h"

/// initialise MCP2515, setup registers. set MCP2515 to normal mode at end
void mcp_init()
{
	mcp_reset();
	
/*	--- Set physical layer config ---
	Fosc		= 16 MHz
	Bus Speed	= 1/(Total # of TQ)*TQ
*/
	const uint8_t BRP		= 1;	// TQ = 2*Tosc*(BRP+1)  = 250 ns
	const uint8_t PrSeg		= 1;	// tPropSeg = (PrSeg + 1)
	const uint8_t PhSeg1	= 2;	// tPS1 = (PhSeg1 + 1)*TQ
	const uint8_t PhSeg2	= 2;	// tPS2 = (PhSeg2 + 1)*TQ
	
	mcp_write(MCP_CNF1, BRP);
	mcp_write(MCP_CNF2, PrSeg | (PhSeg1 << 3));
	mcp_write(MCP_CNF3, PhSeg2);
	
	
	// Setup Tx registers
	//mcp_write(MCP_TXB0SIDL, 0);	  // use only SID 
	//mcp_write(MCP_TXB0DLC, 0x01); // transmit 1 byte, transmit data frame


	// Setup Rx registers
	//mcp_write(MCP_RXM0SIDH, 0);
	//mcp_write(MCP_RXM0SIDL, 0);
	mcp_bitmodify(MCP_RXB0CTRL, 0x60, 0x60);
	mcp_bitmodify(MCP_RXB1CTRL, 0x60, 0x60);	
	
	// go to normal mode
	mcp_bitmodify(MCP_CANCTRL, MODE_MASK, MODE_NORMAL);
	
}


/// reset function for MCP2515
void mcp_reset()
{
	spi_ss_low();
	spi_transmit(MCP_RESET);
	spi_ss_high();
	
	for (uint8_t _ = 0; _ < 128; ++_);	// Wait for 128 cycles
	
	#ifdef MCP_DEBUG
	fprintf(&uart_out, "MCP_RESET\n");
	#endif // MCP_DEBUG
}


/** Read an MCP2515 register
    @param[in] which register adress to read
	@return the values of the read register
*/
uint8_t mcp_read(uint8_t adr)
{
	spi_ss_low();
	spi_transmit(MCP_READ);
	spi_transmit(adr);
	uint8_t read = spi_transmit(0);
	spi_ss_high();
	
	#ifdef MCP_DEBUG
	fprintf(&uart_out, "MCP_READ %x %x\n", adr, read);
	#endif // MCP_DEBUG
	
	return read;
}


/** Read RX0 reciever bufffer of the MCP2515
    \todo implement functionality to select which buffer to read
    @param[in] which buffer to read 
	@return data from buffer
*/
uint8_t mcp_read_buffer(uint8_t rx_select, uint8_t data_select)
{
	spi_ss_low();
	spi_transmit(MCP_READ_RX0 | 0x02);
	uint8_t data = spi_transmit(0);
	spi_ss_high();
	
	mcp_bitmodify(MCP_CANINTF, MCP_RX0IF, 0);
	
	
	#ifdef MCP_DEBUG
	fprintf(&uart_out, "MCP_READ_RX0 %x\n", data);
	#endif // MCP_DEBUG
	
	return data;
}


/** Write data to selected register address
    @param register adress
	@param data values
*/
void mcp_write(uint8_t adr, uint8_t data)
{
	spi_ss_low();
	spi_transmit(MCP_WRITE);
	spi_transmit(adr);
	spi_transmit(data);
	spi_ss_high();
	
	
	#ifdef MCP_DEBUG
	fprintf(&uart_out, "MCP_WRITE\t%x\t%i\n", adr, data);
	#endif // MCP_DEBUG
}

/** MCP ready to trasmit funtion
    @param select buffer
*/
void mcp_rts(uint8_t tx_buffer_select)
{
	spi_ss_low();
	spi_transmit(MCP_RTS | (1<<tx_buffer_select));
	spi_ss_high();
	
	#ifdef MCP_DEBUG
	fprintf(&uart_out, "MCP_RTS\t%i\n", tx_buffer);
	#endif // MCP_DEBUG
}
/** MCP read status function
    @return commonly used status values for the MCP
*/
uint8_t mcp_readstatus()
{
	spi_ss_low();
	spi_transmit(MCP_READ_STATUS);
	uint8_t status = spi_transmit(0);
	spi_ss_high();
	
	
	#ifdef MCP_DEBUG
	fprintf(&uart_out, "MCP_READ_STATUS\t%x\n", status);
	#endif // MCP_DEBUG
	
	return status;
}

/** bit modify command for MCP2515
    @param register adress
	@param mask
	@param data value
*/  
void mcp_bitmodify(uint8_t adr, uint8_t mask, uint8_t data)
{
	spi_ss_low();
	spi_transmit(MCP_BITMOD);
	spi_transmit(adr);
	spi_transmit(mask);
	spi_transmit(data);
	spi_ss_high();
	
	#ifdef MCP_DEBUG
	fprintf(&uart_out, "MCP_BITMOD\t%x\t%x\t%x\n", adr, mask, data);
	#endif // MCP_DEBUG
}

/** Set MCP2515 to loopback mode
*/
void mcp_loopback_set()
{
	mcp_write(MCP_CANCTRL, MODE_CONFIG);
		
	/*	--- Initialize in loopback mode */
	#ifdef MCP_LOOPBACK_INIT
	mcp_write(MCP_CANCTRL, MODE_LOOPBACK);
	// verify
	if (!((mcp_read(MCP_CANSTAT) & MODE_MASK)  == MODE_LOOPBACK))
		fprintf(&uart_out, "Loopback mode set failed\n\tMCP_CANSTAT: 0x%x\n", mcp_read(MCP_CANSTAT));
	#endif // MCP_LOOPBACK_INIT
	
}
