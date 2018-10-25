/*
 * ESP32Encoder.cpp
 *
 *  Created on: Oct 15, 2018
 *      Author: hephaestus
 */

#include <ESP32Encoder.h>
#include <driver/gpio.h>
ESP32Encoder *ESP32Encoder::encoders[MAX_ESP32_ENCODERS] = { NULL, NULL, NULL, NULL,
NULL, NULL, NULL, NULL,
NULL, NULL };


void IRAM_ATTR handleInterrupt0A() {
	ESP32Encoder::encoders[0]->interruptA();
}
void IRAM_ATTR handleInterrupt0B() {
	ESP32Encoder::encoders[0]->interruptB();
}
void IRAM_ATTR handleInterrupt1A() {
	ESP32Encoder::encoders[1]->interruptA();
}
void IRAM_ATTR handleInterrupt1B() {
	ESP32Encoder::encoders[1]->interruptB();
}
void IRAM_ATTR handleInterrupt2A() {
	ESP32Encoder::encoders[2]->interruptA();
}
void IRAM_ATTR handleInterrupt2B() {
	ESP32Encoder::encoders[2]->interruptB();
}
void IRAM_ATTR handleInterrupt3A() {
	ESP32Encoder::encoders[3]->interruptA();
}
void IRAM_ATTR handleInterrupt3B() {
	ESP32Encoder::encoders[3]->interruptB();
}
void IRAM_ATTR handleInterrupt4A() {
	ESP32Encoder::encoders[4]->interruptA();
}
void IRAM_ATTR handleInterrupt4B() {
	ESP32Encoder::encoders[4]->interruptB();
}
void IRAM_ATTR handleInterrupt5A() {
	ESP32Encoder::encoders[5]->interruptA();
}
void IRAM_ATTR handleInterrupt5B() {
	ESP32Encoder::encoders[5]->interruptB();
}
void IRAM_ATTR handleInterrupt6A() {
	ESP32Encoder::encoders[6]->interruptA();
}
void IRAM_ATTR handleInterrupt6B() {
	ESP32Encoder::encoders[6]->interruptB();
}
void IRAM_ATTR handleInterrupt7A() {
	ESP32Encoder::encoders[7]->interruptA();
}
void IRAM_ATTR handleInterrupt7B() {
	ESP32Encoder::encoders[7]->interruptB();
}
void IRAM_ATTR handleInterrupt8A() {
	ESP32Encoder::encoders[8]->interruptA();
}
void IRAM_ATTR handleInterrupt8B() {
	ESP32Encoder::encoders[8]->interruptB();
}
void IRAM_ATTR handleInterrupt9A() {
	ESP32Encoder::encoders[9]->interruptA();
}
void IRAM_ATTR handleInterrupt9B() {
	ESP32Encoder::encoders[9]->interruptB();
}

ESP32Encoder::ESP32Encoder() {
	attached = false;
	aPinNumber = 0;
	bPinNumber = 0;
}

ESP32Encoder::~ESP32Encoder() {
	// TODO Auto-generated destructor stub
}
void ESP32Encoder::attach(int aPinNumber, int bPinNumber, boolean fullQuad) {
	if (attached) {
		Serial.println("All ready attached, FAIL!");
		return;
	}
	int index = 0;
	for (; index < MAX_ESP32_ENCODERS; index++) {
		if (ESP32Encoder::encoders[index] == NULL) {
			encoders[index] = this;
			break;
		}
	}
	if (index == MAX_ESP32_ENCODERS) {
		Serial.println("Too many encoders, FAIL!");
		return;
	}
	// Set data now that pin attach checks are done
	this->aPinNumber = aPinNumber;
	this->bPinNumber = bPinNumber;
	attached = true;
	if (index > 0) {
		for (int i = 0; i < index - 1; i++) {
			//portENTER_CRITICAL_ISR(&encoders[index]->mux);
		}
	}

	pinMode(aPinNumber, INPUT_PULLUP);
	pinMode(bPinNumber, INPUT_PULLUP);
	switch (index) {
	case 0:
		attachInterrupt(digitalPinToInterrupt(aPinNumber), handleInterrupt0A,
		CHANGE);
		if (fullQuad)
			attachInterrupt(digitalPinToInterrupt(bPinNumber),
					handleInterrupt0B, CHANGE);
		break;
	case 1:
		attachInterrupt(digitalPinToInterrupt(aPinNumber), handleInterrupt1A,
		CHANGE);
		if (fullQuad)
			attachInterrupt(digitalPinToInterrupt(bPinNumber),
					handleInterrupt1B, CHANGE);
		break;

	case 2:
		attachInterrupt(digitalPinToInterrupt(aPinNumber), handleInterrupt2A,
		CHANGE);
		if (fullQuad)
			attachInterrupt(digitalPinToInterrupt(bPinNumber),
					handleInterrupt2B, CHANGE);
		break;
	case 3:
		attachInterrupt(digitalPinToInterrupt(aPinNumber), handleInterrupt3A,
		CHANGE);
		if (fullQuad)
			attachInterrupt(digitalPinToInterrupt(bPinNumber),
					handleInterrupt3B, CHANGE);
		break;
	case 4:
		attachInterrupt(digitalPinToInterrupt(aPinNumber), handleInterrupt4A,
		CHANGE);
		if (fullQuad)
			attachInterrupt(digitalPinToInterrupt(bPinNumber),
					handleInterrupt4B, CHANGE);
		break;
	case 5:
		attachInterrupt(digitalPinToInterrupt(aPinNumber), handleInterrupt5A,
		CHANGE);
		if (fullQuad)
			attachInterrupt(digitalPinToInterrupt(bPinNumber),
					handleInterrupt5B, CHANGE);
		break;
	case 6:
		attachInterrupt(digitalPinToInterrupt(aPinNumber), handleInterrupt6A,
		CHANGE);
		if (fullQuad)
			attachInterrupt(digitalPinToInterrupt(bPinNumber),
					handleInterrupt6B, CHANGE);
		break;
	case 7:
		attachInterrupt(digitalPinToInterrupt(aPinNumber), handleInterrupt7A,
		CHANGE);
		if (fullQuad)
			attachInterrupt(digitalPinToInterrupt(bPinNumber),
					handleInterrupt7B, CHANGE);
		break;
	case 8:
		attachInterrupt(digitalPinToInterrupt(aPinNumber), handleInterrupt8A,
		CHANGE);
		if (fullQuad)
			attachInterrupt(digitalPinToInterrupt(bPinNumber),
					handleInterrupt8B, CHANGE);
		break;
	case 9:
		attachInterrupt(digitalPinToInterrupt(aPinNumber), handleInterrupt9A,
		CHANGE);
		if (fullQuad)
			attachInterrupt(digitalPinToInterrupt(bPinNumber),
					handleInterrupt9B, CHANGE);
		break;

	}
	if (index > 0) {
		for (int i = 0; i < index - 1; i++) {
			//portEXIT_CRITICAL_ISR(&encoders[index]->mux);
		}
	}
}

void ESP32Encoder::attachFullQuad(int aPintNumber, int bPinNumber) {
	attach(aPintNumber, bPinNumber, true);
}
void ESP32Encoder::attachHalfQuad(int aPintNumber, int bPinNumber) {
	attach(aPintNumber, bPinNumber, false);
}
void ESP32Encoder::setCount(int32_t value) {
	portENTER_CRITICAL_ISR(&mux);
	count = value;
	portEXIT_CRITICAL_ISR(&mux);
}

int32_t ESP32Encoder::getCount() {
	int32_t tmp = 0;
	portENTER_CRITICAL_ISR(&mux);
	tmp = count;
	portEXIT_CRITICAL_ISR(&mux);
	return tmp;
}
//https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/peripherals/gpio.html#_CPPv214gpio_get_level10gpio_num_t
void ESP32Encoder::interruptA() {
	portENTER_CRITICAL_ISR(&mux);
	if (gpio_get_level((gpio_num_t)aPinNumber) == gpio_get_level((gpio_num_t)bPinNumber)) {
		//portENTER_CRITICAL_ISR(&mux);
		count++;
		//portEXIT_CRITICAL_ISR(&mux);
	} else {
		//portENTER_CRITICAL_ISR(&mux);
		count--;
		//portEXIT_CRITICAL_ISR(&mux);
	}
	portEXIT_CRITICAL_ISR(&mux);
}
void ESP32Encoder::interruptB() {
	portENTER_CRITICAL_ISR(&mux);
	if (gpio_get_level((gpio_num_t)aPinNumber) != gpio_get_level((gpio_num_t)bPinNumber)) {
		//portENTER_CRITICAL_ISR(&mux);
		count++;
		//portEXIT_CRITICAL_ISR(&mux);
	} else {
		//portENTER_CRITICAL_ISR(&mux);
		count--;
		//portEXIT_CRITICAL_ISR(&mux);
	}
	portEXIT_CRITICAL_ISR(&mux);
}
