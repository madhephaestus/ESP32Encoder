#include <ESP32Encoder.h> // https://github.com/madhephaestus/ESP32Encoder.git 
 
#define CLK 13 // CLK ENCODER 
#define DT 15 // DT ENCODER 
 
ESP32Encoder encoder;
 
void setup () { 
  encoder.attachHalfQuad ( DT, CLK );
  encoder.setCount ( 0 );
  Serial.begin ( 115200 );
}
 
void loop () {    
  long newPosition = encoder.getCount() / 2;
  Serial.println(newPosition);
} 
