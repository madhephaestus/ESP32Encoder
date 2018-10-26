/*
 * ESP32Encoder.cpp
 *
 *  Created on: Oct 15, 2018
 *      Author: hephaestus
 */

#include <ESP32Encoder.h>

//static ESP32Encoder *gpio2enc[48];
//
//
ESP32Encoder *ESP32Encoder::encoders[MAX_ESP32_ENCODERS] = { NULL, NULL, NULL,
NULL,
NULL, NULL, NULL, NULL };
//
//
//void IRAM_ATTR handleInterrupt0A() {
//	ESP32Encoder::encoders[0]->interruptA();
//}
//void IRAM_ATTR handleInterrupt0B() {
//	ESP32Encoder::encoders[0]->interruptB();
//}
//void IRAM_ATTR handleInterrupt1A() {
//	ESP32Encoder::encoders[1]->interruptA();
//}
//void IRAM_ATTR handleInterrupt1B() {
//	ESP32Encoder::encoders[1]->interruptB();
//}
//void IRAM_ATTR handleInterrupt2A() {
//	ESP32Encoder::encoders[2]->interruptA();
//}
//void IRAM_ATTR handleInterrupt2B() {
//	ESP32Encoder::encoders[2]->interruptB();
//}
//void IRAM_ATTR handleInterrupt3A() {
//	ESP32Encoder::encoders[3]->interruptA();
//}
//void IRAM_ATTR handleInterrupt3B() {
//	ESP32Encoder::encoders[3]->interruptB();
//}
//void IRAM_ATTR handleInterrupt4A() {
//	ESP32Encoder::encoders[4]->interruptA();
//}
//void IRAM_ATTR handleInterrupt4B() {
//	ESP32Encoder::encoders[4]->interruptB();
//}
//void IRAM_ATTR handleInterrupt5A() {
//	ESP32Encoder::encoders[5]->interruptA();
//}
//void IRAM_ATTR handleInterrupt5B() {
//	ESP32Encoder::encoders[5]->interruptB();
//}
//void IRAM_ATTR handleInterrupt6A() {
//	ESP32Encoder::encoders[6]->interruptA();
//}
//void IRAM_ATTR handleInterrupt6B() {
//	ESP32Encoder::encoders[6]->interruptB();
//}
//void IRAM_ATTR handleInterrupt7A() {
//	ESP32Encoder::encoders[7]->interruptA();
//}
//void IRAM_ATTR handleInterrupt7B() {
//	ESP32Encoder::encoders[7]->interruptB();
//}
//void IRAM_ATTR handleInterrupt8A() {
//	ESP32Encoder::encoders[8]->interruptA();
//}
//void IRAM_ATTR handleInterrupt8B() {
//	ESP32Encoder::encoders[8]->interruptB();
//}
//void IRAM_ATTR handleInterrupt9A() {
//	ESP32Encoder::encoders[9]->interruptA();
//}
//void IRAM_ATTR handleInterrupt9B() {
//	ESP32Encoder::encoders[9]->interruptB();
//}

ESP32Encoder::ESP32Encoder() {
	attached = false;
	aPinNumber = (gpio_num_t) 0;
	bPinNumber = (gpio_num_t) 0;
	working = false;
	direction = false;
}

ESP32Encoder::~ESP32Encoder() {
	// TODO Auto-generated destructor stub
}

//void ESP32Encoder::interrupt(gpio_num_t gpio) {
//    if(working) {
//        if(gpio == aPinNumber && gpio_get_level(aPinNumber))
//            direction = gpio_get_level(bPinNumber);
//        if(direction) {
//            count++;
//        } else {
//            count--;
//        }
//    }
//}

void ESP32Encoder::attach(int a, int b, boolean fq) {
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
	fullQuad = fq;
	// Set data now that pin attach checks are done
	this->aPinNumber = (gpio_num_t) a;
	this->bPinNumber = (gpio_num_t) b;
	gpio_pad_select_gpio(aPinNumber);
	gpio_pad_select_gpio(bPinNumber);
	gpio_set_direction(aPinNumber, GPIO_MODE_OUTPUT);
	gpio_set_direction(bPinNumber, GPIO_MODE_OUTPUT);
	unit = (pcnt_unit_t) index;
	pcnt_config_t r_enc_config;

	r_enc_config	.pulse_gpio_num = aPinNumber; //Rotary Encoder Chan A (GPIO32)
	r_enc_config		.ctrl_gpio_num = bPinNumber;    //Rotary Encoder Chan B (GPIO33)

	r_enc_config		.unit = unit;
	r_enc_config		.channel = PCNT_CHANNEL_0;

	r_enc_config		.pos_mode = fullQuad?PCNT_COUNT_DEC:PCNT_COUNT_DIS; //Count Only On Rising-Edges
	r_enc_config		.neg_mode = PCNT_COUNT_INC;   // Discard Falling-Edge

	r_enc_config		.lctrl_mode = PCNT_MODE_KEEP;    // Rising A on HIGH B = CW Step
	r_enc_config		.hctrl_mode = PCNT_MODE_REVERSE; // Rising A on LOW B = CCW Step

	r_enc_config		.counter_h_lim = INT16_MAX;
	r_enc_config		.counter_l_lim = INT16_MIN ;


	pcnt_unit_config(&r_enc_config);

	pcnt_set_filter_value(unit, 250);  // Filter Runt Pulses
	pcnt_filter_enable(unit);

	gpio_pulldown_en(aPinNumber);
	gpio_pulldown_en(bPinNumber);

	pcnt_counter_pause(unit); // Initial PCNT init
	pcnt_counter_clear(unit);
	pcnt_counter_resume(unit);


}

void ESP32Encoder::attachHalfQuad(int aPintNumber, int bPinNumber){
	attach(aPintNumber, bPinNumber, true);

}
void ESP32Encoder::attachSingleEdge(int aPintNumber, int bPinNumber){
	attach(aPintNumber, bPinNumber, false);
}

void ESP32Encoder::setCount(int32_t value) {
	int16_t c;
	pcnt_get_counter_value(unit, &c);
	count = value - c;
}

int32_t ESP32Encoder::getCount() {
	int16_t c;
	pcnt_get_counter_value(unit, &c);
	return c + count;
}

