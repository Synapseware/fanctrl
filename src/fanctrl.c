#include "fanctrl.h"

volatile uint8_t	tick		= 0;
volatile uint8_t 	adcData		= 0;



//----------------------------------------------------------------
// Gets the MUX configuration bits for the specified channel
static void ConfigureADC(uint8_t channel)
{
	// get the MUX value 
	uint8_t mux =	(ADMUX & 0xF0) |	// mask out the channel bits
					(channel & 0x07);	// set the channel

	switch (channel)
	{
		case 0:
			DIDR0 = (1<<ADC0D);
			DDRC &= ~(1<<PC0);
			break;
		case 1:
			DIDR0 = (1<<ADC1D);
			DDRC &= ~(1<<PC1);
			break;
		case 2:
			DIDR0 = (1<<ADC2D);
			DDRC &= ~(1<<PC2);
			break;
		case 3:
			DIDR0 = (1<<ADC3D);
			DDRC &= ~(1<<PC3);
			break;
		case 4:
			DIDR0 = (1<<ADC4D);
			DDRC &= ~(1<<PC4);
			break;
		case 5:
			DIDR0 = (1<<ADC5D);
			DDRC &= ~(1<<PC5);
			break;
		case 0x0E:	// 1.1v reference
		case 0x0F:	// 0v - GND
			break;
		default:
			return;
	}

	// set the MUX register
	ADMUX = mux;
}


//----------------------------------------------------------------
// Setup ADC to read from the temp sensor.  External analog reference
// voltage is 2.5V
static void initADC(void)
{
	ConfigureADC(ADC_CHANNEL);

	ADMUX	|=	(1<<ADLAR);		// Left adjust result

	ADCSRA	=	(1<<ADEN)	|	// ADC Enable
				(0<<ADSC)	|	// 
				(0<<ADATE)	|	// 
				(0<<ADIF)	|	// 
				(1<<ADIE)	|	// Enable interrupts
				(1<<ADPS2)	|	// Prescaler of 128
				(1<<ADPS1)	|	// 8MHz / 128 = 62.5kHz ADC clock
				(1<<ADPS0);		// 

	ADCSRB	=	(0<<ACME)	|	// Disable the analog comparator
				(0<<ADTS2)	|	// Free running mode
				(0<<ADTS1)	|	// ...
				(0<<ADTS0);		// ...
}


//----------------------------------------------------------------
// Setup timer0
static void initSystemTimer(void)
{
	// FCPU / 256 / 3125 = 10Hz
	// 8MHz / 256 / 3125 = 10

	TCCR1A	=	(0<<COM1A1)	|
				(0<<COM1A0)	|
				(0<<COM1B1)	|
				(0<<COM1B0)	|
				(0<<WGM11)	|	// CTC
				(0<<WGM10);		// CTC

	TCCR1B	=	(0<<ICNC1)	|
				(0<<ICES1)	|
				(0<<WGM13)	|	// CTC
				(1<<WGM12)	|	// CTC
				(1<<CS12)	|	// FCPU/256
				(0<<CS11)	|	// ...
				(0<<CS10);		// ...

	TIMSK1	=	(0<<ICIE1)	|
				(0<<OCIE1B)	|
				(1<<OCIE1A)	|	// Compare A interrupt enable
				(0<<TOIE1);

	OCR1A	=	3125-1;
	OCR1B	=	0xffff;
}


//----------------------------------------------------------------
// Setup the USI for SPI mode
static void initSPI(void)
{
	SPCR	=	(0<<SPIE)	|	// 
				(1<<SPE)	|	// Bit 6 – SPE: SPI Enable
				(0<<DORD)	|	// 
				(1<<MSTR)	|	// Master mode
				(0<<CPOL)	|	// 
				(0<<CPHA)	|	// 
				(1<<SPR1)	|	// Fosc = Fcpu / 64
				(0<<SPR0);		// ...

	SPSR	=	(0<<SPIF)	|	// 
				(0<<WCOL)	|	// 
				(0<<SPI2X);		// No double speed

	// configure the /CS pin for output
	SPI_PORT	|= (1<<SPI_CS) | (1<<SPI_DO) | (1<<SPI_CLK);
	SPI_DDR		|= (1<<SPI_CS) | (1<<SPI_DO) | (1<<SPI_CLK);
}


//----------------------------------------------------------------
// 
static void init(void)
{
    power_adc_enable();
    power_spi_enable();
    power_timer0_enable();
    power_timer1_enable();

	initSystemTimer();

	initADC();

	initSPI();

	sei();
}


//----------------------------------------------------------------
// Converts the value from the temperature sensor to degrees C
static int ConvertToCelcius(int temp)
{
	// Temperature sensor is a TMP35

	// ADC is configured to read the 8 most significant bits only
	// ADC value = (Vin * 1024) / Vref
	//			Vin * 1024
	// result = ----------
	//			   5.0v
	//

	return temp;
}


//----------------------------------------------------------------
// Writes a byte of data to the SPI bus
static uint8_t writeByte(uint8_t data)
{
	// write to the data register
	SPDR = data;

	// wait for data to be transfered
	while(!(SPSR & (1<<SPIF)))
		;

	return SPDR;
}


//----------------------------------------------------------------
// Configures the digital resistor to the given value
static void setResistorValue(int value)
{
	SPI_PORT &= ~(1<<SPI_CS);

	writeByte(MCP_WRITE_WP0);
	writeByte(value >> 1);

	SPI_PORT |= (1<<SPI_CS);
}


//----------------------------------------------------------------
// 
static uint8_t GetLatestAdcData(void)
{
	return adcData;
}


//----------------------------------------------------------------
// Smooths the ADC value by averaging readings
static void SmoothADCValue(uint8_t value)
{
	//adcData = (value>>2) + adcData - (adcData>>2);
	adcData = value;
}


//----------------------------------------------------------------
// 
int main(void)
{
	init();

	// start the first conversion
	ADCSRA |= (1<<ADSC);

	DDRB |= (1<<LED_DBG);
	PORTB &= ~(1<<LED_DBG);

	while(1)
	{
		if (!tick)
			continue;

		// toggle the debug LED
		PINB |= (1<<LED_DBG);

		// convert the ADC data to Celcius
		uint8_t temp = ConvertToCelcius(GetLatestAdcData());

		// update the resistor ladder with new data
		setResistorValue(temp);

		// reset the tick
		tick = 0;
	}

	return 0;
}


//----------------------------------------------------------------
// ADC interrupt complete handler
ISR(ADC_vect)
{
	// just read the ADCH register (top 8 bits)
	SmoothADCValue(ADCH);
}


//----------------------------------------------------------------
// Timer 1 compare A interrupt handler
ISR(TIMER1_COMPA_vect)
{
	tick = 1;

	// start another ADC conversion
	ADCSRA |= (1<<ADSC);
}