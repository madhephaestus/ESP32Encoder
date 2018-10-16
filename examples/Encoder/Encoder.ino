
#include <ESP32Encoder.h>
ESP32Encoder encoder;
ESP32Encoder encoder2;
void setup(){
	Serial.begin(115200);
	Serial.println("Encoder Start = "+String((int32_t)encoder.getCount()));

	encoder.attachFullQuad(36, 39);
	encoder2.attachFullQuad(34, 35);
}

void loop(){
	Serial.println("Encoder count = "+String((int32_t)encoder.getCount())+" "+String((int32_t)encoder2.getCount()));
	delay(100);
}
