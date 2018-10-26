#pragma once
#include <Arduino.h>
#include <driver/gpio.h>
#include "driver/pcnt.h"
#define MAX_ESP32_ENCODERS PCNT_UNIT_MAX
class ESP32Encoder {
private:
	void attach(int aPintNumber, int bPinNumber, boolean fullQuad);
	boolean attached=false;


	static  pcnt_isr_handle_t user_isr_handle; //user's ISR service handle
    bool direction;
    bool working;
public:
	ESP32Encoder();
	~ESP32Encoder();
	void attachHalfQuad(int aPintNumber, int bPinNumber);
	void attachSingleEdge(int aPintNumber, int bPinNumber);
	//void attachHalfQuad(int aPintNumber, int bPinNumber);
	int32_t getCount();
	int32_t getCountRaw();

	boolean isAttached(){return attached;}
	void setCount(int32_t value);
	static ESP32Encoder *encoders[MAX_ESP32_ENCODERS];
	static bool attachedInterrupt;
	gpio_num_t aPinNumber;
	gpio_num_t bPinNumber;
	pcnt_unit_t unit;
	bool fullQuad=false;
	int countsMode = 2;
	volatile int32_t count=0;
	pcnt_config_t r_enc_config;
};
