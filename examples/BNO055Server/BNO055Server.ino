#include <SimplePacketComs.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <BNO055SimplePacketComs.h>
#if defined(ARDUINO_ARCH_ESP32)
	#include <WiFi.h>
	#include <Preferences.h>
	#include <Esp32SimplePacketComs.h>
	#include <wifi/WifiManager.h>
	#include <server/NameCheckerServer.h>
	UDPSimplePacket coms;
	WifiManager manager;
#else
	#include <Teensy32SimplePacketComs.h>
	HIDSimplePacket coms;
#endif

GetIMU * sensor;
long lastPrint = 0;
// Change this to set your team name
String * name = new String("IMU-Team21");


void setup() {
#if defined(ARDUINO_ARCH_ESP32)
	manager.setup();
	coms.attach((PacketEventAbstract *) new NameCheckerServer(name));
#endif
	sensor = new GetIMU();
	sensor->startSensor();
	coms.attach((PacketEventAbstract *) sensor);
}

void loop() {
#if defined(ARDUINO_ARCH_ESP32)
	manager.loop();
	if(manager.getState()==Connected)
#endif
	coms.server();
	sensor->loop();
	if(millis()-lastPrint>100){
		lastPrint=millis();
		//sensor->print();
	}
}


