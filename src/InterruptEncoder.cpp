/*
 * InterruptEncoder.cpp
 *
 *  Created on: Oct 8, 2020
 *      Author: hephaestus
 */
#include "InterruptEncoder.h"
#ifdef ARDUINO
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
int64_t  InterruptEncoder::read() {
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
#else
#include <freertos/FreeRTOS.h>
#include <driver/gpio.h>
#include <esp_timer.h>

#define micros() esp_timer_get_time()
#define digitalRead(pin) gpio_get_level((gpio_num_t)pin)

static void IRAM_ATTR encoderAISR(void * arg) {
    InterruptEncoder * object = (InterruptEncoder *)arg;
    long start = micros();
    long duration = start - object->microsLastA;
    if (duration >= US_DEBOUNCE) {
        object->microsLastA = start;
        object->microsTimeBetweenTicks = duration;
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
    if (attached) {
        gpio_isr_handler_remove((gpio_num_t)apin);
        gpio_uninstall_isr_service();
    }
}
int64_t InterruptEncoder::read() { return count * 2; }
void InterruptEncoder::attach(int aPinNum, int bPinNum) {
    if (attached)
        return;
    apin = aPinNum;
    bpin = bPinNum;

    gpio_reset_pin((gpio_num_t)apin);
    gpio_set_direction((gpio_num_t)apin, GPIO_MODE_INPUT);
    gpio_set_level((gpio_num_t)apin, GPIO_PULLUP_ONLY);
    gpio_set_intr_type((gpio_num_t)apin, GPIO_INTR_ANYEDGE);

    gpio_install_isr_service(0);
    gpio_isr_handler_add((gpio_num_t)apin, encoderAISR, (void *)this);
}
#endif
