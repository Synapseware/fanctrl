#ifndef __SEPIC_H__
#define __SEPIC_H__


#include <avr/io.h>
#include <avr/interrupt.h>
#include <inttypes.h>
#include <avr/sleep.h>
#include <avr/power.h>


#define F_MAX		255

const float			SCALE		= (float)(F_MAX/255.0);
const uint8_t		MAX			= (uint8_t)(F_MAX * 0.90);

#define	ADC_IN		ADC3D
#define ADC_PIN		PB3


#define PWM_PIN		PB4


#endif
