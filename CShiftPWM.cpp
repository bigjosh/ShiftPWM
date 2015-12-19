/*
CShiftPWM.cpp - ShiftPWM.h - Library for Arduino to PWM many outputs using shift registers
Copyright (c) 2011-2012 Elco Jacobs, www.elcojacobs.com
Modifications (c) 2015 Josh Levine www.josh.com
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

/* workaround for a bug in WString.h */
#define F(string_literal) (reinterpret_cast<const __FlashStringHelper *>(PSTR(string_literal)))

#include "CShiftPWM.h"
#include <Arduino.h>

CShiftPWM::CShiftPWM() :   // Constants are set in initializer list

	m_latchPin(h_latchPin) , 
	m_dataPin(h_dataPin) , 
	m_clockPin(h_clockPin) 
    
{
									  
	m_maxBrightness = 0;
	m_counter = 0;
	m_pinGrouping = 1; // Default = RGBRGBRGB... PinGrouping = 3 means: RRRGGGBBBRRRGGGBBB...

	m_amountOfRegisters = h_amountOfRegisters;
	m_amountOfOutputs = h_amountOfOutputs;
	m_PWMValues = h_PWMValues;							// The actual storage is allocated in ShiftPWM.h as h_PWMValues
		
}

CShiftPWM::~CShiftPWM() {
	// TODO: Disable INT
}

bool CShiftPWM::IsValidPin(int pin){
	if(pin<m_amountOfOutputs){
		return 1;
	}
	else {
		return 0;
	}
}


void CShiftPWM::SetOne(int pin, unsigned char value){
	if(IsValidPin(pin) ){
		m_PWMValues[pin]=value;
	}
}

void CShiftPWM::SetAll(unsigned char value){
	for(int k=0 ; k<(m_amountOfOutputs);k++){
		m_PWMValues[k]=value;
	}
}

void CShiftPWM::SetGroupOf2(int group, unsigned char v0,unsigned char v1, int offset){
	int skip = m_pinGrouping*(group/m_pinGrouping); // is not equal to 2*group. Division is rounded down first.
	if(IsValidPin(group+skip+offset+m_pinGrouping) ){
		m_PWMValues[group+skip+offset]					=v0;
		m_PWMValues[group+skip+offset+m_pinGrouping]	=v1;
	}
}

void CShiftPWM::SetGroupOf3(int group, unsigned char v0,unsigned char v1,unsigned char v2, int offset){
	int skip = 2*m_pinGrouping*(group/m_pinGrouping); // is not equal to 2*group. Division is rounded down first.
	if(IsValidPin(group+skip+offset+2*m_pinGrouping) ){
		m_PWMValues[group+skip+offset]					=v0;
		m_PWMValues[group+skip+offset+m_pinGrouping]	=v1;
		m_PWMValues[group+skip+offset+m_pinGrouping*2]	=v2;
	}
}

void CShiftPWM::SetGroupOf4(int group, unsigned char v0,unsigned char v1,unsigned char v2,unsigned char v3, int offset){
	int skip = 3*m_pinGrouping*(group/m_pinGrouping); // is not equal to 2*group. Division is rounded down first.
	if(IsValidPin(group+skip+offset+3*m_pinGrouping) ){
		m_PWMValues[group+skip+offset]					=v0;
		m_PWMValues[group+skip+offset+m_pinGrouping]	=v1;
		m_PWMValues[group+skip+offset+m_pinGrouping*2]	=v2;
		m_PWMValues[group+skip+offset+m_pinGrouping*3]	=v3;
	}
}

void CShiftPWM::SetGroupOf5(int group, unsigned char v0,unsigned char v1,unsigned char v2,unsigned char v3,unsigned char v4, int offset){
	int skip = 4*m_pinGrouping*(group/m_pinGrouping); // is not equal to 2*group. Division is rounded down first.
	if(IsValidPin(group+skip+offset+4*m_pinGrouping) ){
		m_PWMValues[group+skip+offset]					=v0;
		m_PWMValues[group+skip+offset+m_pinGrouping]	=v1;
		m_PWMValues[group+skip+offset+m_pinGrouping*2]	=v2;
		m_PWMValues[group+skip+offset+m_pinGrouping*3]	=v3;
		m_PWMValues[group+skip+offset+m_pinGrouping*4]	=v4;
	}
}

void CShiftPWM::SetRGB(int led, unsigned char r,unsigned char g,unsigned char b, int offset){
	int skip = 2*m_pinGrouping*(led/m_pinGrouping); // is not equal to 2*led. Division is rounded down first.
	if(IsValidPin(led+skip+offset+2*m_pinGrouping) ){
		m_PWMValues[led+skip+offset]					=( (unsigned int) r * m_maxBrightness)>>8;
		m_PWMValues[led+skip+offset+m_pinGrouping]		=( (unsigned int) g * m_maxBrightness)>>8;
		m_PWMValues[led+skip+offset+2*m_pinGrouping]	=( (unsigned int) b * m_maxBrightness)>>8;
	}
}

void CShiftPWM::SetAllRGB(unsigned char r,unsigned char g,unsigned char b){
	for(int k=0 ; (k+3*m_pinGrouping-1) < m_amountOfOutputs; k+=3*m_pinGrouping){
		for(int l=0; l<m_pinGrouping;l++){
			m_PWMValues[k+l]				=	( (unsigned int) r * m_maxBrightness)>>8;
			m_PWMValues[k+l+m_pinGrouping]	=	( (unsigned int) g * m_maxBrightness)>>8;
			m_PWMValues[k+l+m_pinGrouping*2]	=	( (unsigned int) b * m_maxBrightness)>>8;
		}
	}
}

void CShiftPWM::SetHSV(int led, unsigned int hue, unsigned int sat, unsigned int val, int offset){
	unsigned char r,g,b;
	unsigned int H_accent = hue/60;
	unsigned int bottom = ((255 - sat) * val)>>8;
	unsigned int top = val;
	unsigned char rising  = ((top-bottom)  *(hue%60   )  )  /  60  +  bottom;
	unsigned char falling = ((top-bottom)  *(60-hue%60)  )  /  60  +  bottom;

	switch(H_accent) {
	case 0:
		r = top;
		g = rising;
		b = bottom;
		break;

	case 1:
		r = falling;
		g = top;
		b = bottom;
		break;

	case 2:
		r = bottom;
		g = top;
		b = rising;
		break;

	case 3:
		r = bottom;
		g = falling;
		b = top;
		break;

	case 4:
		r = rising;
		g = bottom;
		b = top;
		break;

	case 5:
		r = top;
		g = bottom;
		b = falling;
		break;
	}
	SetRGB(led,r,g,b,offset);
}

void CShiftPWM::SetAllHSV(unsigned int hue, unsigned int sat, unsigned int val){
	// Set the first LED
	SetHSV(0, hue, sat, val);
	// Copy RGB values all LED's.
	SetAllRGB(m_PWMValues[0],m_PWMValues[m_pinGrouping],m_PWMValues[2*m_pinGrouping]);
}

// OneByOne functions are usefull for testing all your outputs
void CShiftPWM::OneByOneSlow(void){
	OneByOne_core(1024/m_maxBrightness);
}

void CShiftPWM::OneByOneFast(void){
	OneByOne_core(1);
}

void CShiftPWM::OneByOne_core(int delaytime){
	int pin,brightness;
	SetAll(0);
	for(int pin=0;pin<m_amountOfOutputs;pin++){
		for(brightness=0;brightness<m_maxBrightness;brightness++){
			m_PWMValues[pin]=brightness;
			delay(delaytime);
		}
		for(brightness=m_maxBrightness;brightness>=0;brightness--){
			m_PWMValues[pin]=brightness;
			delay(delaytime);
		}
	}
}


void CShiftPWM::SetPinGrouping(int grouping){
	// Sets the number of pins per color that are used after eachother. RRRRGGGGBBBBRRRRGGGGBBBB would be a grouping of 4.
	m_pinGrouping = grouping;
}



void CShiftPWM::Start(int ledFrequency, unsigned char maxBrightness){
	// Configure and enable timer1 or timer 2 for a compare and match A interrupt.
	m_maxBrightness = maxBrightness;

	pinMode(m_dataPin, OUTPUT);
	pinMode(m_clockPin, OUTPUT);
	pinMode(m_latchPin, OUTPUT);

	digitalWrite(m_clockPin, LOW);
	digitalWrite(m_dataPin, LOW);
	
	m_irqPrescaler = (ledFrequency / 1000)+1;		// We are running off the 1ms interrupt. +1 becuase we pre-increment inside the ISR
		
	// Enable a match interrupt on Timer0 which is already running at 1KHz for the Arduino time functions
	// This trick lifted from....
	// https://learn.adafruit.com/multi-tasking-the-arduino-part-2/timers
	
	// enable timer 0 compare match interrupt
	// portable definitions taken from...
	// https://github.com/arduino/Arduino/blob/master/hardware/arduino/avr/cores/arduino/wiring.c
	
	#if defined(TIMSK) && defined(TOIE0)
		bitSet(TIMSK, OCIE0A);
	#elif defined(TIMSK0) && defined(TOIE0)
		bitSet(TIMSK0, OCIE0A);
	#else
		#error	Timer 0 overflow interrupt not set correctly
	#endif	
}

void CShiftPWM::Stop(void) {
	
	// disable timer 0 compare match interrupt
	#if defined(TIMSK) && defined(TOIE0)
		bitClear(TIMSK, OCIE0A);
	#elif defined(TIMSK0) && defined(TOIE0)
		bitClear(TIMSK0, OCIE0A);
	#else
		#error	Timer 0 overflow interrupt not set correctly
	#endif	
	
}
