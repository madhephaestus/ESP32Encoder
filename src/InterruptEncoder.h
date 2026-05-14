/*
 * InterruptEncoder.h
 *
 *  Created on: Oct 8, 2020
 *      Author: hephaestus
 */

#ifndef INTERRUPTENCODER_H_
#define INTERRUPTENCODER_H_

#define MAX_ENCODERS 16
#define US_DEBOUNCE 10

#ifdef ARDUINO
#include <Arduino.h>
#else 
#include <stdint.h>
#endif

#ifdef ESP32ENCODER_USE_INT32
typedef int32_t enc_count_t;
#else
typedef int64_t enc_count_t;
#endif

class InterruptEncoder {
private:
bool attached=false;



public:

	int apin=0;
	int bpin=0;
	InterruptEncoder();
	virtual ~InterruptEncoder();
	void attach(int aPinNum, int bPinNum);
	volatile bool aState=0;
	volatile bool bState=0;
	volatile enc_count_t count=0;
	volatile int64_t microsLastA=0;
	volatile int64_t microsTimeBetweenTicks=0;
	enc_count_t read();
};

#endif /* INTERRUPTENCODER_H_ */
