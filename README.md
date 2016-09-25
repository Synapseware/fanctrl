Fan Controller
==============

Computer case fan speed controller, which uses a simple "fixed" buck converter for controller fan voltage.

PWM is not ideal for DC brushless fans, especially if you want to get constant data from the speed sensor.  This circuit forms a very simple buck converter by using a high-switching frequency coupled with the standard inductor, capacitor and diode.


# ADC Input
The ADC is configured for free-running mode.  The input to the ADC should pass through an RC filter or an RLC filter for improved noise immunity.

## Input Pin
The ADC is configured to read from PB4.

## ADC Sense Line
The input to the ADC should be constrained to 0.0v through 1.1v.  This gives the most accuracy on the ADC and allows for a small load/sense resistor.

# PWM Output
Timer1 is used in a Fast-PWM configuration with the 64mHz PLL clock.  This allows for a maximum of 250kHz switching frequency.  Relatively low by today's standards but very workable on a proto board or hand-made board.

## PWM Output Pin
The PWM output is available on PB1.  The output should not be filtered since it must be fed to the SMPS switching element.  Care should be taken to keep this line short and include an appropriately sized pull-down/pull-up resistor to ensure safe operation of the power MOSFET or other switching transistor.

# Other Chip Information
The AVR uC is configured to use it's internal R/C oscillator.  High clock accuracy is not needed by this circuit and keeping the part count low is a goal of this example.


# Build
This project compiles and uploads just fine using Linux and avr-gnu.
