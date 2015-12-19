/*
ShiftPWM.h - Library for Arduino to PWM many outputs using shift registers
Copyright (c) 2011-2012 Elco Jacobs, www.elcojacobs.com
All right reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/


#ifndef ShiftPWM_H
#define ShiftPWM_H

#include "pins_arduino_compile_time.h" // My own version of pins arduino, which does not define the arrays in program memory
#include <Arduino.h>
#include "CShiftPWM.h"

// These are all defined inside the header file so that we can statically allocate the memory for the values array
// Because it s statically allocated, it will show up as memory used at compile time rather than sielently failing
// if we run out of memory at run time. 

const unsigned char h_amountOfRegisters = ShiftPWM_numRegisters;
const int h_amountOfOutputs = h_amountOfRegisters*8;
unsigned char h_PWMValues[h_amountOfOutputs];

const int h_latchPin = ShiftPWM_latchPin;
const int h_dataPin  = ShiftPWM_dataPin;
const int h_clockPin = ShiftPWM_clockPin;

// The ShiftPWM object is created in the header file, instead of defining it as extern here and creating it in the cpp file.
// If the ShiftPWM object is created in the cpp file, it is separately compiled with the library.
// The compiler cannot treat it as constant and cannot optimize well: it will generate many memory accesses in the interrupt function.

CShiftPWM ShiftPWM;

// The inline function below uses normal output pins to send one bit to the SPI port.
// This function is used in the noSPI mode and is useful if you need the SPI port for something else.
// It is a lot 2.5x slower than the SPI version.
static inline void pwm_output_one_pin(volatile uint8_t * const clockPort, volatile uint8_t * const dataPort,\
                                  const uint8_t clockBit, const uint8_t dataBit, \
                                  unsigned char counter, unsigned char * ledPtr){
    bitClear(*clockPort, clockBit);
    if(ShiftPWM_invertOutputs){
      bitWrite(*dataPort, dataBit, *(ledPtr)<=counter );
    }
    else{
      bitWrite(*dataPort, dataBit, *(ledPtr)>counter );
    }
    bitSet(*clockPort, clockBit);
}

// More portable vector definaition taken from...
// https://github.com/arduino/Arduino/blob/master/hardware/arduino/avr/cores/arduino/wiring.c


#if defined(__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
ISR(TIM0_COMPA_vect)
#else
ISR(TIMER0_COMPA_vect)
#endif
	
	{
		
	volatile static unsigned char skipCounter=1;			// Always run the 1st pass though
	
	if (--skipCounter == 0 ) {					// Should we run this pass?
				
		skipCounter = ShiftPWM.m_irqPrescaler;	// Reset for next time
		
		volatile static unsigned char semaphore = 0;		// Don't step on ourselves
		
		if (!semaphore) {
			
			semaphore=1;
	
	
			sei(); //enable interrupt nesting to prevent disturbing other interrupt functions (servo's for example).

			// Look up which bit of which output register corresponds to the pin.
			// This should be constant, so the compiler can optimize this code away and use sbi and cbi instructions
			// The compiler only knows this if this function is compiled in the same file as the pin setting.
			// That is the reason the full funcion is in the header file, instead of only the prototype.
			// If this function is defined in cpp files of the library, it is compiled seperately from the main file.
			// The compiler does not recognize the pins/ports as constant and sbi and cbi instructions cannot be used.


			volatile uint8_t * const latchPort = port_to_output_PGM_ct[digital_pin_to_port_PGM_ct[ShiftPWM_latchPin]];
			const uint8_t latchBit =  digital_pin_to_bit_PGM_ct[ShiftPWM_latchPin];

			volatile uint8_t * const clockPort = port_to_output_PGM_ct[digital_pin_to_port_PGM_ct[ShiftPWM_clockPin]];
			volatile uint8_t * const dataPort  = port_to_output_PGM_ct[digital_pin_to_port_PGM_ct[ShiftPWM_dataPin]];
			const uint8_t clockBit =  digital_pin_to_bit_PGM_ct[ShiftPWM_clockPin];
			const uint8_t dataBit =   digital_pin_to_bit_PGM_ct[ShiftPWM_dataPin];   

			// Define a pointer that will be used to access the values for each output. 
			// Start it one past the last value, because it is decreased before it is used.

			unsigned char * ledPtr= ShiftPWM.m_PWMValues + ShiftPWM.m_amountOfOutputs;

			// Write shift register latch clock low 
			bitClear(*latchPort, latchBit);
			unsigned char counter = ShiftPWM.m_counter;
	
			//Use port manipulation to send out all bits
			for(unsigned char i = ShiftPWM.m_amountOfRegisters; i>0;--i){   // do one shift register at a time. This unrolls the loop for extra speed
				if(ShiftPWM_balanceLoad){
					counter +=8; // distribute the load by using a shifted counter per shift register
				}
				pwm_output_one_pin(clockPort, dataPort, clockBit, dataBit, counter, --ledPtr);  // This takes 12 or 13 clockcycles
				pwm_output_one_pin(clockPort, dataPort, clockBit, dataBit, counter, --ledPtr);
				pwm_output_one_pin(clockPort, dataPort, clockBit, dataBit, counter, --ledPtr);
				pwm_output_one_pin(clockPort, dataPort, clockBit, dataBit, counter, --ledPtr);
				pwm_output_one_pin(clockPort, dataPort, clockBit, dataBit, counter, --ledPtr);
				pwm_output_one_pin(clockPort, dataPort, clockBit, dataBit, counter, --ledPtr);
				pwm_output_one_pin(clockPort, dataPort, clockBit, dataBit, counter, --ledPtr);
				pwm_output_one_pin(clockPort, dataPort, clockBit, dataBit, counter, --ledPtr);
							//digitalWrite(13,1)	;
							//digitalWrite(13,0)	;
			}

			// Write shift register latch clock high
			bitSet(*latchPort, latchBit);

			if(ShiftPWM.m_counter<ShiftPWM.m_maxBrightness){
				ShiftPWM.m_counter++; // Increase the counter
			}
			else{
				ShiftPWM.m_counter=0; // Reset counter if it maximum brightness has been reached
			}

			
			semaphore=0;

		}
		
	}
}

// #endif for include once.
#endif
