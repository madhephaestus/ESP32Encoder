#pragma once
#include <Arduino.h>
#define MAX_ESP32_ENCODERS 10
class ESP32Encoder {
private:
	void attach(int aPintNumber, int bPinNumber, boolean fullQuad);
	portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
	boolean attached=false;
	int aPinNumber;
	int bPinNumber;
	volatile int32_t count=0;
public:
	ESP32Encoder();
	~ESP32Encoder();
	void attachFullQuad(int aPintNumber, int bPinNumber);
	void attachHalfQuad(int aPintNumber, int bPinNumber);
	int32_t getCount();
	void interruptA();
	void interruptB();
	boolean isAttached(){return attached;}
	void setCount(int32_t value);
	static ESP32Encoder *encoders[MAX_ESP32_ENCODERS];


};
