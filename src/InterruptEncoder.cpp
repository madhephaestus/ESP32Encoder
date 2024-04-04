/*
 * InterruptEncoder.cpp
 *
 *  Created on: Oct 8, 2020
 *      Author: hephaestus
 */
#include "InterruptEncoder.h"
void IRAM_ATTR encoderAISR(void * arg) {
	InterruptEncoder* object=(InterruptEncoder*)arg;
	long start = micros();
	long duration=start - object->microsLastA;
	if (duration >= US_DEBOUNCE) {
		object->microsLastA = start;
		object->microsTimeBetweenTicks=duration;
		object->aState = digitalRead(object->apin);
		object->bState = digitalRead(object->bpin);
		if (object->aState == object->bState)
			object->count = object->count + 1;
		else
			object->count = object->count - 1;
	}
}
InterruptEncoder::InterruptEncoder() {}
InterruptEncoder::~InterruptEncoder() {
	if(attached)
		detachInterrupt(digitalPinToInterrupt(apin));
}
int64_t  InterruptEncoder::read(){
	return count*2;
}
void InterruptEncoder::attach(int aPinNum, int bPinNum) {
	if(attached)
		return;
	apin = aPinNum;
	bpin = bPinNum;
	pinMode(apin, INPUT_PULLUP);
	pinMode(bpin, INPUT_PULLUP);
	attachInterruptArg(digitalPinToInterrupt(apin), encoderAISR,this, CHANGE);
}

