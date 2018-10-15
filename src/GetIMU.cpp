/*
 * GetIMU.cpp
 *
 *  Created on: Jun 7, 2018
 *      Author: hephaestus
 */

#include "GetIMU.h"
float printData[15];
#include <Wire.h>
//User function to be called when a packet comes in
// Buffer contains data from the packet cming in at the start of the function
// User data is written into the buffer to send it back
void GetIMU::event(float * buffer) {
	if(!started)
		return;
	buffer[0] = a.x();
	buffer[1] = a.y();
	buffer[2] = a.z();
	buffer[3] = v.x();
	buffer[4] = v.y();
	buffer[5] = v.z();

	buffer[6] = g.x();
	buffer[7] = g.y();
	buffer[8] = g.z();

	buffer[9] = e.z();
	buffer[10] = e.y();
	buffer[11] = e.x();
}
void GetIMU::loop() {
	if(!started)
		return;
#if defined(ARDUINO_ARCH_ESP32)
	Wire.reset();
#endif
	e = bno->getVector(Adafruit_BNO055::VECTOR_EULER);
	a = bno->getVector(Adafruit_BNO055::VECTOR_LINEARACCEL);
	v = bno->getVector(Adafruit_BNO055::VECTOR_GYROSCOPE);
	g = bno->getVector(Adafruit_BNO055::VECTOR_GRAVITY);


}
void GetIMU::startSensor(){
	Wire.reset();
	if(bno==NULL)
		bno = new Adafruit_BNO055();
	started=bno->begin(Adafruit_BNO055::OPERATION_MODE_NDOF_FMC_OFF);

	if(started)
		bno->setExtCrystalUse(true);
}
void GetIMU::print(){
	event(printData);
	Serial.print("\r\n Acceleration= "
			+String(printData[0])+" , "
			+String(printData[1])+" , "
			+String(printData[2])+"\r\n Gyro= "
			+String(printData[3])+" , "
			+String(printData[4])+" , "
			+String(printData[5])+"\r\n Gravity= "
			+String(printData[6])+" , "
			+String(printData[7])+" , "
			+String(printData[8])+"\r\n Euler= "
			+String(printData[9])+" , "
			+String(printData[10])+" , "
			+String(printData[11])
			);
	  /* Display calibration status for each sensor. */
	  uint8_t system, gyro, accel, mag = 0;
	  bno->getCalibration(&system, &gyro, &accel, &mag);
	  Serial.print("\r\n CALIBRATION: Sys=");
	  Serial.print(system, DEC);
	  Serial.print(" Gyro=");
	  Serial.print(gyro, DEC);
	  Serial.print(" Accel=");
	  Serial.print(accel, DEC);
	  Serial.print(" Mag=");
	  Serial.println(mag, DEC);
}
