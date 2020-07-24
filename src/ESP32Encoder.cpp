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
enum puType ESP32Encoder::useInternalWeakPullResistors=DOWN;
ESP32Encoder *ESP32Encoder::encoders[MAX_ESP32_ENCODERS] = { NULL, NULL, NULL,
NULL,
NULL, NULL, NULL, NULL };

bool ESP32Encoder::attachedInterrupt=false;
pcnt_isr_handle_t ESP32Encoder::user_isr_handle = NULL;

ESP32Encoder::ESP32Encoder() {
	attached = false;
	aPinNumber = (gpio_num_t) 0;
	bPinNumber = (gpio_num_t) 0;
	working = false;
	direction = false;
	unit = (pcnt_unit_t) -1;
}

ESP32Encoder::~ESP32Encoder() {
}

/* Decode what PCNT's unit originated an interrupt
 * and pass this information together with the event type
 * the main program using a queue.
 */
static void IRAM_ATTR pcnt_example_intr_handler(void *arg) {
	ESP32Encoder * ptr;

	uint32_t intr_status = PCNT.int_st.val;
	int i;

	for (i = 0; i < PCNT_UNIT_MAX; i++) {
		if (intr_status & (BIT(i))) {
			ptr = ESP32Encoder::encoders[i];
			/* Save the PCNT event type that caused an interrupt
			 to pass it to the main program */

			int64_t status=0;
			if(PCNT.status_unit[i].h_lim_lat){
				status=ptr->r_enc_config.counter_h_lim;
			}
			if(PCNT.status_unit[i].l_lim_lat){
				status=ptr->r_enc_config.counter_l_lim;
			}
			//pcnt_counter_clear(ptr->unit);
			PCNT.int_clr.val = BIT(i); // clear the interrupt
			ptr->count = status + ptr->count;
		}
	}
}

void ESP32Encoder::attach(int a, int b, enum encType et) {
	if (attached) {
		Serial.println("Already attached, FAIL!");
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
	pcnt_set_filter_value(unit, 250);  // Filter Runt Pulses
	pcnt_filter_enable(unit);


	/* Enable events on maximum and minimum limit values */
	pcnt_event_enable(unit, PCNT_EVT_H_LIM);
	pcnt_event_enable(unit, PCNT_EVT_L_LIM);

	pcnt_counter_pause(unit); // Initial PCNT init
	pcnt_counter_clear(unit);
	/* Register ISR handler and enable interrupts for PCNT unit */
	if(attachedInterrupt==false){
		attachedInterrupt=true;
		esp_err_t er = pcnt_isr_register(pcnt_example_intr_handler,(void *) NULL, (int)0,
				(pcnt_isr_handle_t *)&ESP32Encoder::user_isr_handle);
		if (er != ESP_OK){
			Serial.println("Encoder wrap interrupt failed");
		}
	}
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
	return getCountRaw() + count;
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

