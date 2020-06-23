#pragma once
#include <Arduino.h>
#include <driver/gpio.h>
#include "driver/pcnt.h"
#define MAX_ESP32_ENCODERS PCNT_UNIT_MAX
#define 	_INT16_MAX 32766
#define  	_INT16_MIN -32766

enum encType {
single,
half,
full
};
enum puType {
UP,
DOWN,
NONE
};
class ESP32Encoder {
private:
	void attach(int aPintNumber, int bPinNumber, enum encType et);
	boolean attached=false;


	static  pcnt_isr_handle_t user_isr_handle; //user's ISR service handle
    bool direction;
    bool working;

	static bool attachedInterrupt;
	int64_t getCountRaw();
public:
	ESP32Encoder();
	~ESP32Encoder();
	void attachHalfQuad(int aPintNumber, int bPinNumber);
	void attachFullQuad(int aPintNumber, int bPinNumber);
	void attachSingleEdge(int aPintNumber, int bPinNumber);
	//void attachHalfQuad(int aPintNumber, int bPinNumber);
	int64_t getCount();
	int64_t clearCount();
	int64_t pauseCount();
	int64_t resumeCount();

	boolean isAttached(){return attached;}
	void setCount(int64_t value);
	static ESP32Encoder *encoders[MAX_ESP32_ENCODERS];
	gpio_num_t aPinNumber;
	gpio_num_t bPinNumber;
	pcnt_unit_t unit;
	bool fullQuad=false;
	int countsMode = 2;
	volatile int64_t count=0;
	pcnt_config_t r_enc_config;
	static enum puType useInternalWeakPullResistors;
};

//Added by Sloeber 
#pragma once

