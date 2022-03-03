/*
 * ESP32Encoder.cpp
 *
 *  Created on: Oct 15, 2018
 *      Author: hephaestus
 */

#include <ESP32Encoder.h>
#include <soc/pcnt_struct.h>
#include "esp_log.h"

static const char* TAG = "ESP32Encoder";


//static ESP32Encoder *gpio2enc[48];
//
//
enum puType ESP32Encoder::useInternalWeakPullResistors=DOWN;
ESP32Encoder *ESP32Encoder::encoders[MAX_ESP32_ENCODERS] = { NULL, };

bool ESP32Encoder::attachedInterrupt=false;
pcnt_isr_handle_t ESP32Encoder::user_isr_handle = NULL;

ESP32Encoder::ESP32Encoder(bool always_interrupt_, enc_isr_cb_t enc_isr_cb, void* enc_isr_cb_data):
	always_interrupt{always_interrupt_},
	aPinNumber{(gpio_num_t) 0},
	bPinNumber{(gpio_num_t) 0},
	unit{(pcnt_unit_t) -1},
	fullQuad{false},
	countsMode{2},
	count{0},
	r_enc_config{},
	_enc_isr_cb(enc_isr_cb),
	_enc_isr_cb_data(enc_isr_cb_data),
	attached{false},
	direction{false},
	working{false}
{
}

ESP32Encoder::~ESP32Encoder() {}

/* Decode what PCNT's unit originated an interrupt
 * and pass this information together with the event type
 * the main program using a queue.
 */
#ifdef CONFIG_IDF_TARGET_ESP32S2
	#define COUNTER_H_LIM cnt_thr_h_lim_lat_un
	#define COUNTER_L_LIM cnt_thr_l_lim_lat_un
#else
	#define COUNTER_H_LIM h_lim_lat
	#define COUNTER_L_LIM l_lim_lat
#endif

static void IRAM_ATTR esp32encoder_pcnt_intr_handler(void *arg) {
	ESP32Encoder * esp32enc = {};
	uint32_t intr_status = PCNT.int_st.val;
	for (uint8_t i = 0; i < PCNT_UNIT_MAX; i++) {
		if (intr_status & (BIT(i))) {
			pcnt_unit_t unit = static_cast<pcnt_unit_t>(i);
			esp32enc = ESP32Encoder::encoders[i];
			if(PCNT.status_unit[i].COUNTER_H_LIM){
				esp32enc->count += esp32enc->r_enc_config.counter_h_lim;
				pcnt_counter_clear(unit);
			} else if(PCNT.status_unit[i].COUNTER_L_LIM){
				esp32enc->count += esp32enc->r_enc_config.counter_l_lim;
				pcnt_counter_clear(unit);
			} else if(esp32enc->always_interrupt && (PCNT.status_unit[i].thres0_lat || PCNT.status_unit[i].thres1_lat)) {
				int16_t c;
				pcnt_get_counter_value(unit, &c);
				esp32enc->count += c;
				pcnt_set_event_value(unit, PCNT_EVT_THRES_0, -1);
				pcnt_set_event_value(unit, PCNT_EVT_THRES_1, 1);
				pcnt_event_enable(unit, PCNT_EVT_THRES_0);
				pcnt_event_enable(unit, PCNT_EVT_THRES_1);
				pcnt_counter_clear(unit);
				if (esp32enc->_enc_isr_cb) {
					esp32enc->_enc_isr_cb(esp32enc->_enc_isr_cb_data);
				}
			}
			PCNT.int_clr.val = BIT(i); // clear the interrupt
		}
	}
}

void ESP32Encoder::detatch(){
	pcnt_counter_pause(unit);
	ESP32Encoder::encoders[unit]=NULL;

}
void ESP32Encoder::attach(int a, int b, enum encType et) {
	if (attached) {
		ESP_LOGE(TAG, "attach: already attached");
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
		ESP_LOGE(TAG, "Too many encoders, FAIL!");
		return;
	}

	// Set data now that pin attach checks are done
	fullQuad = et != single;
		unit = (pcnt_unit_t) index;
	this->aPinNumber = (gpio_num_t) a;
	this->bPinNumber = (gpio_num_t) b;

	//Set up the IO state of hte pin
	gpio_pad_select_gpio(aPinNumber);
	gpio_pad_select_gpio(bPinNumber);
	gpio_set_direction(aPinNumber, GPIO_MODE_INPUT);
	gpio_set_direction(bPinNumber, GPIO_MODE_INPUT);
	if(useInternalWeakPullResistors==DOWN){
		gpio_pulldown_en(aPinNumber);
		gpio_pulldown_en(bPinNumber);
	}
	if(useInternalWeakPullResistors==UP){
		gpio_pullup_en(aPinNumber);
		gpio_pullup_en(bPinNumber);
	}
	// Set up encoder PCNT configuration
	r_enc_config.pulse_gpio_num = aPinNumber; //Rotary Encoder Chan A
	r_enc_config.ctrl_gpio_num = bPinNumber;    //Rotary Encoder Chan B

	r_enc_config.unit = unit;
	r_enc_config.channel = PCNT_CHANNEL_0;

	r_enc_config.pos_mode = fullQuad ? PCNT_COUNT_DEC : PCNT_COUNT_DIS; //Count Only On Rising-Edges
	r_enc_config.neg_mode = PCNT_COUNT_INC;   // Discard Falling-Edge

	r_enc_config.lctrl_mode = PCNT_MODE_KEEP;    // Rising A on HIGH B = CW Step
	r_enc_config.hctrl_mode = PCNT_MODE_REVERSE; // Rising A on LOW B = CCW Step

	r_enc_config		.counter_h_lim = _INT16_MAX;
	r_enc_config		.counter_l_lim = _INT16_MIN ;

	pcnt_unit_config(&r_enc_config);

	if (et == full) {
		// set up second channel for full quad
		r_enc_config.pulse_gpio_num = bPinNumber; //make prior control into signal
		r_enc_config.ctrl_gpio_num = aPinNumber;    //and prior signal into control

		r_enc_config.unit = unit;
		r_enc_config.channel = PCNT_CHANNEL_1; // channel 1

		r_enc_config.pos_mode = PCNT_COUNT_DEC; //Count Only On Rising-Edges
		r_enc_config.neg_mode = PCNT_COUNT_INC;   // Discard Falling-Edge

		r_enc_config.lctrl_mode = PCNT_MODE_REVERSE;    // prior high mode is now low
		r_enc_config.hctrl_mode = PCNT_MODE_KEEP; // prior low mode is now high

		r_enc_config		.counter_h_lim = _INT16_MAX;
		r_enc_config		.counter_l_lim = _INT16_MIN ;

		pcnt_unit_config(&r_enc_config);
	} else { // make sure channel 1 is not set when not full quad
		r_enc_config.pulse_gpio_num = bPinNumber; //make prior control into signal
		r_enc_config.ctrl_gpio_num = aPinNumber;    //and prior signal into control

		r_enc_config.unit = unit;
		r_enc_config.channel = PCNT_CHANNEL_1; // channel 1

		r_enc_config.pos_mode = PCNT_COUNT_DIS; //disabling channel 1
		r_enc_config.neg_mode = PCNT_COUNT_DIS;   // disabling channel 1

		r_enc_config.lctrl_mode = PCNT_MODE_DISABLE;    // disabling channel 1
		r_enc_config.hctrl_mode = PCNT_MODE_DISABLE; // disabling channel 1

		r_enc_config		.counter_h_lim = _INT16_MAX;
		r_enc_config		.counter_l_lim = _INT16_MIN ;

		pcnt_unit_config(&r_enc_config);
	}

	// Filter out bounces and noise
	setFilter(250); // Filter Runt Pulses

	/* Enable events on maximum and minimum limit values */
	pcnt_event_enable(unit, PCNT_EVT_H_LIM);
	pcnt_event_enable(unit, PCNT_EVT_L_LIM);
	pcnt_counter_pause(unit); // Initial PCNT init
	/* Register ISR handler and enable interrupts for PCNT unit */
	if(! attachedInterrupt){
		if (always_interrupt) {
			pcnt_set_event_value(unit, PCNT_EVT_THRES_0, -1);
			pcnt_set_event_value(unit, PCNT_EVT_THRES_1, 1);
			pcnt_event_enable(unit, PCNT_EVT_THRES_0);
			pcnt_event_enable(unit, PCNT_EVT_THRES_1);
		}
		esp_err_t er = pcnt_isr_register(esp32encoder_pcnt_intr_handler,(void *) NULL, (int)0,
				(pcnt_isr_handle_t *)&ESP32Encoder::user_isr_handle);
		if (er != ESP_OK){
			ESP_LOGE(TAG, "Encoder wrap interrupt failed");
		}
		attachedInterrupt=true;
	}
	pcnt_counter_clear(unit);
	pcnt_intr_enable(unit);
	pcnt_counter_resume(unit);

}

void ESP32Encoder::attachHalfQuad(int aPintNumber, int bPinNumber) {
	attach(aPintNumber, bPinNumber, half);

}
void ESP32Encoder::attachSingleEdge(int aPintNumber, int bPinNumber) {
	attach(aPintNumber, bPinNumber, single);
}
void ESP32Encoder::attachFullQuad(int aPintNumber, int bPinNumber) {
	attach(aPintNumber, bPinNumber, full);
}

void ESP32Encoder::setCount(int64_t value) {
	count = value - getCountRaw();
}
int64_t ESP32Encoder::getCountRaw() {
	int16_t c;
	pcnt_get_counter_value(unit, &c);
	return c;
}
int64_t ESP32Encoder::getCount() {
	return count + getCountRaw();
}

int64_t ESP32Encoder::clearCount() {
	count = 0;
	return pcnt_counter_clear(unit);
}

int64_t ESP32Encoder::pauseCount() {
	return pcnt_counter_pause(unit);
}

int64_t ESP32Encoder::resumeCount() {
	return pcnt_counter_resume(unit);
}

void ESP32Encoder::setFilter(uint16_t value) {
	if(value>1023)value=1023;
	if(value==0) {
		pcnt_filter_disable(unit);
	} else {
		pcnt_set_filter_value(unit, value);
		pcnt_filter_enable(unit);
	}

}
