#include "fanctrl.h"

volatile int lastADC	= 0;
volatile uint8_t tick	= 0;


//----------------------------------------------------------------
// Setup ADC to read from the temp sensor.  External analog reference
// voltage is 2.5V
static void initADC(void)
{
	ADMUX	=	(0<<REFS1)	|	// ARef (2.5V)
				(0<<REFS0)	|	// ARef 
				(0<<ADLAR)	|	// No left adjust
				(0<<MUX3)	|	// ADC0
				(0<<MUX2)	|	// ...
				(0<<MUX1)	|	// ...
				(0<<MUX0);		// ...

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

	DIDR0	=	(1<<ADC_IN);	// Analog input on ADC0
	DDRC	&=	~(1<<ADC_PIN);	// Set pin as input
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
static uint8_t ConvertTempToCelcius(int temp)
{
	// Temperature sensor is a TMP35
	// Scale is 10mV/C
	// Range is 10c to 125c
	// 10c	=  100mV
	// 25c	=  250mV
	// 90c	=  900mV
	// 100c	= 1000mV

	// ADC is configured to read the 8 most significant bits only
	// ADC value = (Vin * 1024) / Vref
	//			Vin * 1024
	// result = ----------
	//			   2.5v
	//

	return 255 - ((uint8_t) temp);
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
static void setResistorValue(uint8_t value)
{
	SPI_PORT &= ~(1<<SPI_CS);

	writeByte(MCP_WRITE_WP0);
	writeByte(value);

	SPI_PORT |= (1<<SPI_CS);
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

		PINB |= (1<<LED_DBG);

		uint8_t temp = ConvertTempToCelcius(lastADC);

		// dump the last ADC reading to the digital pot
		setResistorValue(temp);

		// reset the ticker
		tick = 0;
	}

	return 0;
}


//----------------------------------------------------------------
// ADC interrupt complete handler
ISR(ADC_vect)
{
	// read the full ADC value
	lastADC	= ADC;
}


//----------------------------------------------------------------
// Timer 1 compare A interrupt handler
ISR(TIMER1_COMPA_vect)
{
	//PINB |= (1<<LED_DBG);

	tick = 1;

	// start another ADC conversion
	ADCSRA |= (1<<ADSC);
}