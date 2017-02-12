#include "fanctrl.h"

static uint8_t lastADC = 255;
static uint8_t tick = 0;

//----------------------------------------------------------------
// Setup ADC to read from the temp sensor
static void initADC(void)
{
	ADMUX	=   (0<<REFS2)  |   // Vcc as Vref
                (0<<REFS1)	|	// ...
				(0<<REFS0)	|	// ...
				(1<<ADLAR)	|	// Left adjust so we can read ADCH only
				(0<<MUX3)	|	// MUX3:0 = 0010 => ADC3 (PB3)
				(0<<MUX2)	|	// ...
				(1<<MUX1)	|	// ...
				(1<<MUX0);		// ...

	ADCSRA	=   (1<<ADEN)	|	// Enable ADC
				(0<<ADSC)	|	// Don't start a conversion just yet
				(1<<ADATE)	|	// Enable auto-triggering
				(1<<ADIF)	|	// Clear any previous interrupt
				(1<<ADIE)	|	// Enable ADC interrupts
				(1<<ADPS2)	|	// Max prescaler for fast CPU clock
				(1<<ADPS1)	|	// ...
				(1<<ADPS0);		// ...

	ADCSRB	=   (0<<BIN)	|	// No-bipolar input mode
				(0<<IPR)	|	// don't reverse input polarity
				(0<<ADTS2)	|	// free-running
				(0<<ADTS1)	|	// ...
				(0<<ADTS0);		// ...

	DIDR0	=   (1<<ADC_IN);    // disable digital input on ADC pin
	DDRB	&=  ~(1<<ADC_PIN);  // set ADC pin as input
}


//----------------------------------------------------------------
// Setup timer0 for 1mS
static void initSystemTimer(void)
{
	TCCR0A	=	(0<<COM0A1)	|	// 
				(0<<COM0A0)	|	// 
				(0<<COM0B1)	|	// 
				(0<<COM0B0)	|	// 
				(1<<WGM01)	|	// CTC
				(0<<WGM00);		// CTC

	TCCR0B	=	(0<<FOC0A)	|	// 
				(0<<FOC0B)	|	// 
				(0<<WGM02)	|	// CTC
				(1<<CS02)	|	// Fcpu/256
				(0<<CS01)	|	// ...
				(0<<CS00);		// ...

	TIMSK	|=	(1<<OCIE0A)	|	// Overflow on CTC
				(0<<OCIE0B)	|	// 
				(0<<TOIE0);		// 

	OCR0A	=	249;			// 1mS cycle
}


//----------------------------------------------------------------
// Setup the USI for SPI mode
static void initSPI(void)
{
	USICR	=	(0<<USISIE) |	// 
				(0<<USIOIE) |	// 
				(0<<USIWM1) |	// 
				(1<<USIWM0) |	// 3-wire mode
				(0<<USICS1) |	// 
				(0<<USICS0) |	// 
				(1<<USICLK) |	// software strobe
				(0<<USITC);		// 


	// configure the /CS pin for output
	PORTB |= (1<<SPI_CS);
	DDRB |= (1<<SPI_CS);
}


//----------------------------------------------------------------
// 
static void init(void)
{
    power_adc_enable();
    power_timer0_enable();
    power_timer1_enable();

    TIMSK	= 0;
    GTCCR	= 0;

	initSystemTimer();

	initADC();

	initSPI();

	sei();
}


//----------------------------------------------------------------
// Writes a byte of data to the SPI bus
static uint8_t writeByte(uint8_t data)
{
	USIDR	=	data;
	data	=	(1<<USIOIF);
	USISR	=	data;

	data	=	(1<<USIWM0) |
				(1<<USICS1) |
				(1<<USICLK) |
				(1<<USITC);

	// clock out the data
	while(1)
	{
		USICR = data;
		if ((USISR & (1<<USIOIF)) != 0)
			break;
	}

	// return the data
	return USIDR;
}


//----------------------------------------------------------------
// Configures the digital resistor to the given value
static void setResistorValue(uint8_t value)
{
	PORTB &= ~(1<<SPI_CS);

	writeByte(MCP_WRITE_WP0);
	writeByte(value);

	PORTB |= (1<<SPI_CS);
}


//----------------------------------------------------------------
// 
int main(void)
{
	init();

	// start the first conversion
	ADCSRA |= (1<<ADSC);

	while(1)
	{
		if (!tick)
			continue;

		// dump the last ADC reading to the digital pot
		// smarts to come later...
		setResistorValue(lastADC);

		tick = 0;
	}

	return 0;
}


//----------------------------------------------------------------
// ADC interrupt complete handler
ISR(ADC_vect)
{
	lastADC	= ADCH;
}


//----------------------------------------------------------------
// TIMER0 COMPA interrupt handler
ISR(TIM0_COMPA_vect)
{
	static uint8_t counter = 0;

	counter++;
	if (counter < 10)
		return;

	tick = 1;
	counter = 0;
}