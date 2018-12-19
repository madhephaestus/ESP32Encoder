
#include <ESP32Encoder.h>
ESP32Encoder encoder;
ESP32Encoder encoder2;
void setup(){
	Serial.begin(115200);
	encoder.setCount(37);// set starting count value
	encoder2.setCount(42);// set starting count value
	Serial.println("Encoder Start = "+String((int32_t)encoder.getCount()));

	// Attache pins for use as encoder pins
	encoder.attachHalfQuad(36, 39);
	// Attache pins for use as encoder pins
	encoder2.attachHalfQuad(34, 35);
}

void loop(){
	// Loop and read the count
	Serial.println("Encoder count = "+String((int32_t)encoder.getCount())+" "+String((int32_t)encoder2.getCount()));
	delay(100);
}
