#include "fanctrl.h"

//----------------------------------------------------------------
// Fast PWM on OC1B/PB2
static void initPWM(void)
{
	TCCR1	=   (1<<CTC1)	|
				(1<<PWM1A)  |
				(0<<COM1A1) |
				(0<<COM1A0) |
				(0<<CS13)	|		// clk/16
				(1<<CS12)	|
				(0<<CS11)	|
				(1<<CS10);

	GTCCR	|=  (1<<PWM1B)  |
				(1<<COM1B1) |       // enable OC1B
				(0<<COM1B0) |
				(0<<FOC1B)  |
				(0<<FOC1A)  |
				(0<<PSR1);

	PLLCSR	=   (0<<LSM)	|
				(1<<PCKE)	|		// use 64MHz PLL for clock source
				(1<<PLLE)	|
				(0<<PLOCK);

	OCR1C	=	F_MAX;		        // in this mode, counter restarts on OCR1C
	OCR1B	=	0;		        	// PWM value

	DDRB	|=	(1<<PWM_PIN);	    // enable OC1B I/O
}


//----------------------------------------------------------------
// Slow PWM on OC0A/PB0
static void initPWMAlt(void)
{
	GTCCR	=	(0<<TSM)	|
				(0<<PSR0);

	TCCR0A	=	(1<<COM0A1)	|	// 
				(0<<COM0A0)	|
				(0<<COM0B1)	|
				(0<<COM0B0)	|
				(1<<WGM01)	|	// FastPWM
				(1<<WGM00);

	TCCR0B	=	(0<<FOC0A)	|
				(0<<FOC0B)	|
				(0<<WGM02)	|	// FastPWM
				(0<<CS02)	|
				(0<<CS01)	|
				(0<<CS00);

	TIMSK	=	(0<<OCIE0A)	|
				(0<<OCIE0B)	|
				(0<<TOIE0);

}


//----------------------------------------------------------------
// Setup ADC to read trimpot on ADC2/PB4
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
// 
static void init(void)
{
    power_adc_enable();
    power_timer0_enable();
    power_timer1_enable();

	initPWM();

	initADC();

	sei();
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
		// NO-OP
	}

	return 0;
}


//----------------------------------------------------------------
// ADC interrupt complete handler
ISR(ADC_vect)
{
	uint8_t sample	= ADCH;

	uint8_t result = (uint8_t) ((float)sample * SCALE);

	// set bounds
	if (result > MAX)
		result = MAX;
	else if (result < MIN)
		result = MIN;

	// take ADC reading and set PWM output to that value
	OCR1B = result;
}
