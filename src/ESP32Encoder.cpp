/*
 * ESP32Encoder.cpp
 *
 *  Created on: Oct 15, 2018
 *      Author: hephaestus
 */

#include <ESP32Encoder.h>
#include <driver/gpio.h>
ESP32Encoder *ESP32Encoder::encoders[MAX_ESP32_ENCODERS] = { NULL, NULL, NULL,
NULL,
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

//static void IRAM_ATTR __onEncoderInterruptA(void *arg) {
//	if (arg == NULL)
//		return;
//	ESP32Encoder * ptr = (ESP32Encoder*) arg;
//	uint32_t amask = (uint32_t) 1 << ptr->aPinNumber;
//
//	uint32_t gpio_intr_status_l = 0;
//	uint32_t gpio_intr_status_h = 0;
//
//	gpio_intr_status_l = GPIO.status;
//	gpio_intr_status_h = GPIO.status1.val;
//
//	//GPIO.status_w1tc = gpio_intr_status_l;//Clear intr for gpio0-gpio31
//	//GPIO.status1_w1tc.val = gpio_intr_status_h;//Clear intr for gpio32-39
//
//	if (gpio_intr_status_l & amask) {
//		GPIO.status_w1tc = amask;
//		ptr->interruptA();
//	}
//	amask = ((uint32_t) 1 << (ptr->aPinNumber - 32));
//
//	if (gpio_intr_status_h & amask) {
//		GPIO.status1_w1tc.val = amask;
//		ptr->interruptA();
//	}
//}
//static void IRAM_ATTR __onEncoderInterruptB(void *arg) {
//	if (arg == NULL)
//		return;
//	ESP32Encoder * ptr = (ESP32Encoder*) arg;
//	uint32_t amask = (uint32_t) 1 << ptr->bPinNumber;
//
//	uint32_t gpio_intr_status_l = 0;
//	uint32_t gpio_intr_status_h = 0;
//
//	gpio_intr_status_l = GPIO.status;
//	gpio_intr_status_h = GPIO.status1.val;
//
//	//GPIO.status_w1tc = gpio_intr_status_l;//Clear intr for gpio0-gpio31
//	//GPIO.status1_w1tc.val = gpio_intr_status_h;//Clear intr for gpio32-39
//
//	if (gpio_intr_status_l & amask) {
//		GPIO.status_w1tc = amask;
//		ptr->interruptB();
//	}
//	amask = ((uint32_t) 1 << (ptr->bPinNumber - 32));
//
//	if (gpio_intr_status_h & amask) {
//		GPIO.status1_w1tc.val = amask;
//		ptr->interruptB();
//	}
//}

ESP32Encoder::ESP32Encoder() {
	attached = false;
	aPinNumber = 0;
	bPinNumber = 0;
}

ESP32Encoder::~ESP32Encoder() {
	// TODO Auto-generated destructor stub
}
void ESP32Encoder::attach(int aPinNumber, int bPinNumber, boolean fq) {
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

	pinMode(aPinNumber, INPUT_PULLUP);
	pinMode(bPinNumber, INPUT_PULLUP);
	astate = digitalRead(aPinNumber) != 0;
	bstate = digitalRead(bPinNumber) != 0;
	this->fullQuad=fq;
//	esp_err_t err = esp_intr_alloc(ETS_GPIO_INTR_SOURCE,
//			(int) ESP_INTR_FLAG_IRAM, (intr_handler_t) __onEncoderInterruptA,
//			(void *) this, (intr_handle_t *) &gpio_intr_handle);
//	if (err == ESP_ERR_INVALID_ARG) {
//		Serial.println("Error, ESP_ERR_INVALID_ARG! " + String(err));
//		return;
//	}
//	if (err == ESP_ERR_NOT_FOUND) {
//		Serial.println(
//				"Error ESP_ERR_NOT_FOUND, no more interrupts availible! "
//						+ String(err));
//		return;
//	}

//	esp_intr_disable(gpio_intr_handle);
//	if (esp_intr_get_cpu(gpio_intr_handle)) { //APP_CPU
//		GPIO.pin[aPinNumber].int_ena = 1;
//	} else { //PRO_CPU
//		GPIO.pin[aPinNumber].int_ena = 4;
//	}
//	GPIO.pin[aPinNumber].int_type = CHANGE;
//	esp_intr_enable(gpio_intr_handle);
//
//	if (fullQuad) {
//		err = esp_intr_alloc(ETS_GPIO_INTR_SOURCE,
//				(int) ESP_INTR_FLAG_IRAM ,
//				(intr_handler_t) __onEncoderInterruptB, (void *) this,
//				(intr_handle_t *) &gpio_intr_handle);
//		if (err != ESP_OK) {
//			Serial.println(
//					"Error, no more interrupts availible! " + String(err));
//			return;
//		}
//		esp_intr_disable(gpio_intr_handle);
//		if (esp_intr_get_cpu(gpio_intr_handle)) { //APP_CPU
//			GPIO.pin[bPinNumber].int_ena = 1;
//		} else { //PRO_CPU
//			GPIO.pin[bPinNumber].int_ena = 4;
//		}
//		GPIO.pin[bPinNumber].int_type = CHANGE;
//		esp_intr_enable(gpio_intr_handle);
//
//	}
//	return;

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

}

void ESP32Encoder::attachFullQuad(int aPintNumber, int bPinNumber) {
	attach(aPintNumber, bPinNumber, true);
}
void ESP32Encoder::attachHalfQuad(int aPintNumber, int bPinNumber) {
	attach(aPintNumber, bPinNumber, false);
}
void ESP32Encoder::setCount(int32_t value) {
	//portENTER_CRITICAL_ISR(&mux);
	count = value;
	//portEXIT_CRITICAL_ISR(&mux);
}

int32_t ESP32Encoder::getCount() {
	int32_t tmp = 0;
	//portENTER_CRITICAL_ISR(&mux);
	tmp = count;
	//portEXIT_CRITICAL_ISR(&mux);
	return tmp;
}

int ESP32Encoder::gpio_get(int gpio_num) {
	int tmp = 0;
	tmp = gpio_get_level((gpio_num_t) gpio_num);
	//tmp = digitalRead(gpio_num);
	return tmp;
}

//https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/peripherals/gpio.html#_CPPv214gpio_get_level10gpio_num_t
void ESP32Encoder::interruptA() {
	//portENTER_CRITICAL_ISR(&mux);
	astate = gpio_get(aPinNumber) != 0;
	if (!fullQuad)
		bstate = gpio_get(bPinNumber) != 0;
	if (astate == bstate) {
		//portENTER_CRITICAL_ISR(&mux);
		count++;
		//portEXIT_CRITICAL_ISR(&mux);
	} else {
		//portENTER_CRITICAL_ISR(&mux);
		count--;
		//portEXIT_CRITICAL_ISR(&mux);
	}
	//portEXIT_CRITICAL_ISR(&mux);
}
void ESP32Encoder::interruptB() {
	bstate = gpio_get(bPinNumber) != 0;

	//portENTER_CRITICAL_ISR(&mux);
	if (astate != bstate) {
		//portENTER_CRITICAL_ISR(&mux);
		count++;
		//portEXIT_CRITICAL_ISR(&mux);
	} else {
		//portENTER_CRITICAL_ISR(&mux);
		count--;
		//portEXIT_CRITICAL_ISR(&mux);
	}
	//portEXIT_CRITICAL_ISR(&mux);
}
