#include "fanctrl.h"

//----------------------------------------------------------------
// Fast PWM on OC1A/PB1
static void initPWM(void)
{
	TCCR1	=   (1<<CTC1)	|
				(1<<PWM1A)  |
				(0<<COM1A1) |
				(0<<COM1A0) |
				(0<<CS13)	|		// clk/1
				(1<<CS12)	|
				(1<<CS11)	|
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
	OCR1B	=	F_MAX;		        // PWM value

	DDRB	|=	(1<<PWM_PIN);	    // enable OC1B I/O
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

	uint8_t result = sample; // (uint8_t) (sample * SCALE);

	// set bounds
    /*
	if (result > MAX)
		result = MAX;
	else if (result < 1)
		result = 1;
    */

	// take ADC reading and set PWM output to that value
	OCR1B = result;
}
