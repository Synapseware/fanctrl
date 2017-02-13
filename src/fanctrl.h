#ifndef __SEPIC_H__
#define __SEPIC_H__


#include <avr/io.h>
#include <avr/interrupt.h>
#include <inttypes.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <util/delay.h>


//--------------------------------------------------
// Board constants
#define	ADC_IN		ADC3D
#define ADC_PIN		PB3

#define SPI_CS		PB4
#define SPI_DO		PB1
#define SPI_DI		PB0
#define SPI_CLK		PB2

//--------------------------------------------------
// MCP41x1 driver data
#define MCP_A3		7
#define MCP_A2		6
#define MCP_A1		5
#define MCP_A0		4
#define MCP_C1		3
#define MCP_C0		2
#define MCP_D1		1
#define MCP_D0		0

#define MCP_WIPER_0		0x00
#define MCP_WIPER_1		0x01
#define MCP_TCON_REG	0x04
#define MCP_STATUS_REG	0x05

#define MCP_WRITE_WP0	0b00000000
#define MCP_READ_WP0	0b00001100
#define MCP_INC_WP0		0b00000100
#define MCP_DEC_WP0		0b00001000

#define MCP_WRITE_WP1	0b00010000
#define MCP_READ_WP1	0b00011100
#define MCP_INC_WP1		0b00010100
#define MCP_DEC_WP1		0b00011000


#endif
