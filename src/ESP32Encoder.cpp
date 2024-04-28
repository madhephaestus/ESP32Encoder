/*
 * ESP32Encoder.cpp
 *
 *  Created on: Oct 15, 2018
 *      Author: hephaestus
 */

#include <ESP32Encoder.h>
#include <Arduino.h>
#include <soc/soc_caps.h>
#if SOC_PCNT_SUPPORTED
// Not all esp32 chips support the pcnt (notably the esp32c3 does not)
#include <soc/pcnt_struct.h>
#include "esp_log.h"
#include "esp_ipc.h"
#if ( defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3) )
	#include <freertos/FreeRTOS.h>
	#include <rom/gpio.h>
#endif

static const char* TAG_ENCODER = "ESP32Encoder";

static portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED;
#define _ENTER_CRITICAL() portENTER_CRITICAL_SAFE(&spinlock)
#define _EXIT_CRITICAL() portEXIT_CRITICAL_SAFE(&spinlock)


//static ESP32Encoder *gpio2enc[48];
//
//
puType ESP32Encoder::useInternalWeakPullResistors = puType::down;
uint32_t ESP32Encoder::isrServiceCpuCore = ISR_CORE_USE_DEFAULT;
ESP32Encoder *ESP32Encoder::encoders[MAX_ESP32_ENCODERS] = { NULL, };

bool ESP32Encoder::attachedInterrupt=false;

ESP32Encoder::ESP32Encoder(bool always_interrupt_, enc_isr_cb_t enc_isr_cb, void* enc_isr_cb_data):
	always_interrupt{always_interrupt_},
	aPinNumber{(gpio_num_t) 0},
	bPinNumber{(gpio_num_t) 0},
	unit{(pcnt_unit_t) -1},
	countsMode{2},
	count{0},
	r_enc_config{},
	_enc_isr_cb(enc_isr_cb),
	_enc_isr_cb_data(enc_isr_cb_data),
	attached{false},
	direction{false},
	working{false}
{
	if (enc_isr_cb_data == nullptr)
	{
		_enc_isr_cb_data = this;
	}
}

ESP32Encoder::~ESP32Encoder() {}

/* Decode what PCNT's unit originated an interrupt
 * and pass this information together with the event type
 * the main program using a queue.
 */
#ifdef CONFIG_IDF_TARGET_ESP32S2
	#define COUNTER_H_LIM cnt_thr_h_lim_lat_un
	#define COUNTER_L_LIM cnt_thr_l_lim_lat_un
	#define thres0_lat cnt_thr_thres0_lat_un
	#define thres1_lat cnt_thr_thres1_lat_un

#elif CONFIG_IDF_TARGET_ESP32S3
	#define COUNTER_H_LIM cnt_thr_h_lim_lat_un
	#define COUNTER_L_LIM cnt_thr_l_lim_lat_un
	#define thres0_lat cnt_thr_thres0_lat_un
	#define thres1_lat cnt_thr_thres1_lat_un
#else
	#define COUNTER_H_LIM h_lim_lat
	#define COUNTER_L_LIM l_lim_lat
#endif



static void esp32encoder_pcnt_intr_handler(void *arg) {
	ESP32Encoder * esp32enc = static_cast<ESP32Encoder *>(arg);
	pcnt_unit_t unit = esp32enc->r_enc_config.unit;
	_ENTER_CRITICAL();
	if(PCNT.status_unit[unit].COUNTER_H_LIM){
		esp32enc->count = esp32enc->count + esp32enc->r_enc_config.counter_h_lim;
		pcnt_counter_clear(unit);
	} else if(PCNT.status_unit[unit].COUNTER_L_LIM){
		esp32enc->count = esp32enc->count + esp32enc->r_enc_config.counter_l_lim;
		pcnt_counter_clear(unit);
	} else if(esp32enc->always_interrupt && (PCNT.status_unit[unit].thres0_lat || PCNT.status_unit[unit].thres1_lat)) {
		int16_t c;
		pcnt_get_counter_value(unit, &c);
		esp32enc->count = esp32enc->count + c;
		pcnt_set_event_value(unit, PCNT_EVT_THRES_0, -1);
		pcnt_set_event_value(unit, PCNT_EVT_THRES_1, 1);
		pcnt_event_enable(unit, PCNT_EVT_THRES_0);
		pcnt_event_enable(unit, PCNT_EVT_THRES_1);
		pcnt_counter_clear(unit);
		if (esp32enc->_enc_isr_cb) {
			esp32enc->_enc_isr_cb(esp32enc->_enc_isr_cb_data);
		}
	}
	_EXIT_CRITICAL();
}





void ESP32Encoder::detach(){
	pcnt_counter_pause(unit);
	pcnt_isr_handler_remove(this->r_enc_config.unit);
	ESP32Encoder::encoders[unit]=NULL;
	attached = false;
}

void ESP32Encoder::detatch(){
	this->detach();
}

static IRAM_ATTR void ipc_install_isr_on_core(void *arg) {
    esp_err_t *result = (esp_err_t*) arg;
    *result = pcnt_isr_service_install(0);
}

void ESP32Encoder::attach(int a, int b, encType et) {
	if (attached) {
		ESP_LOGE(TAG_ENCODER, "attach: already attached");
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
		while(1){
			ESP_LOGE(TAG_ENCODER, "Too many encoders, FAIL!");
			delay(100);
		}
		
	}

	// Set data now that pin attach checks are done
	unit = (pcnt_unit_t) index;
	this->aPinNumber = (gpio_num_t) a;
	this->bPinNumber = (gpio_num_t) b;

	//Set up the IO state of hte pin
	gpio_pad_select_gpio(aPinNumber);
	gpio_pad_select_gpio(bPinNumber);
	gpio_set_direction(aPinNumber, GPIO_MODE_INPUT);
	gpio_set_direction(bPinNumber, GPIO_MODE_INPUT);
	if(useInternalWeakPullResistors == puType::down){
		gpio_pulldown_en(aPinNumber);
		gpio_pulldown_en(bPinNumber);
	}
	if(useInternalWeakPullResistors == puType::up){
		gpio_pullup_en(aPinNumber);
		gpio_pullup_en(bPinNumber);
	}
	// Set up encoder PCNT configuration
	// Configure channel 0
	r_enc_config.pulse_gpio_num = aPinNumber; //Rotary Encoder Chan A
	r_enc_config.ctrl_gpio_num = bPinNumber;    //Rotary Encoder Chan B

	r_enc_config.unit = unit;
	r_enc_config.channel = PCNT_CHANNEL_0;

	r_enc_config.pos_mode = et != encType::single ? PCNT_COUNT_DEC : PCNT_COUNT_DIS; //Count Only On Rising-Edges
	r_enc_config.neg_mode = PCNT_COUNT_INC;   // Discard Falling-Edge

	r_enc_config.lctrl_mode = PCNT_MODE_KEEP;    // Rising A on HIGH B = CW Step
	r_enc_config.hctrl_mode = PCNT_MODE_REVERSE; // Rising A on LOW B = CCW Step

	r_enc_config		.counter_h_lim = _INT16_MAX;
	r_enc_config		.counter_l_lim = _INT16_MIN ;

	pcnt_unit_config(&r_enc_config);

	// Configure channel 0
	r_enc_config.pulse_gpio_num = bPinNumber; //make prior control into signal
	r_enc_config.ctrl_gpio_num = aPinNumber;    //and prior signal into control

	r_enc_config.channel = PCNT_CHANNEL_1; // channel 1

	r_enc_config.pos_mode = PCNT_COUNT_DIS; //disabling channel 1
	r_enc_config.neg_mode = PCNT_COUNT_DIS;   // disabling channel 1

	r_enc_config.lctrl_mode = PCNT_MODE_DISABLE;    // disabling channel 1
	r_enc_config.hctrl_mode = PCNT_MODE_DISABLE; // disabling channel 1

	if (et == encType::full) {
		// set up second channel for full quad

		r_enc_config.pos_mode = PCNT_COUNT_DEC; //Count Only On Rising-Edges
		r_enc_config.neg_mode = PCNT_COUNT_INC;   // Discard Falling-Edge

		r_enc_config.lctrl_mode = PCNT_MODE_REVERSE;    // prior high mode is now low
		r_enc_config.hctrl_mode = PCNT_MODE_KEEP; // prior low mode is now high
	}
	pcnt_unit_config(&r_enc_config);

	// Filter out bounces and noise
	setFilter(250); // Filter Runt Pulses

	/* Enable events on maximum and minimum limit values */
	pcnt_event_enable(unit, PCNT_EVT_H_LIM);
	pcnt_event_enable(unit, PCNT_EVT_L_LIM);
	pcnt_counter_pause(unit); // Initial PCNT init
	/* Register ISR service and enable interrupts for PCNT unit */
	if(! attachedInterrupt){
#ifdef CONFIG_IDF_TARGET_ESP32S2 // esp32-s2 is single core, no ipc call
			esp_err_t er = pcnt_isr_service_install(0);
			if (er != ESP_OK){
				ESP_LOGE(TAG_ENCODER, "Encoder install isr service failed");
			}
#else
		if (isrServiceCpuCore == ISR_CORE_USE_DEFAULT || isrServiceCpuCore == xPortGetCoreID()) {
			esp_err_t er = pcnt_isr_service_install(0);
			if (er != ESP_OK){
				ESP_LOGE(TAG_ENCODER, "Encoder install isr service on same core failed");
			}
		} else {
			esp_err_t ipc_ret_code = ESP_FAIL;
			esp_err_t er = esp_ipc_call_blocking(isrServiceCpuCore, ipc_install_isr_on_core, &ipc_ret_code);
			if (er != ESP_OK){
				ESP_LOGE(TAG_ENCODER, "IPC call to install isr service on core %d failed", isrServiceCpuCore);
			}
			if (ipc_ret_code != ESP_OK){
				ESP_LOGE(TAG_ENCODER, "Encoder install isr service on core %d failed", isrServiceCpuCore);
			}
		}
#endif

		attachedInterrupt=true;
	}

	// Add ISR handler for this unit
	if (pcnt_isr_handler_add(unit, esp32encoder_pcnt_intr_handler, this) != ESP_OK) {
		ESP_LOGE(TAG_ENCODER, "Encoder install interrupt handler for unit %d failed", unit);
	}

	if (always_interrupt){
		pcnt_set_event_value(unit, PCNT_EVT_THRES_0, -1);
		pcnt_set_event_value(unit, PCNT_EVT_THRES_1, 1);
		pcnt_event_enable(unit, PCNT_EVT_THRES_0);
		pcnt_event_enable(unit, PCNT_EVT_THRES_1);
	}
	pcnt_counter_clear(unit);
	pcnt_intr_enable(unit);
	pcnt_counter_resume(unit);

	attached = true;

}

void ESP32Encoder::attachHalfQuad(int aPintNumber, int bPinNumber) {
	attach(aPintNumber, bPinNumber, encType::half);

}
void ESP32Encoder::attachSingleEdge(int aPintNumber, int bPinNumber) {
	attach(aPintNumber, bPinNumber, encType::single);
}
void ESP32Encoder::attachFullQuad(int aPintNumber, int bPinNumber) {
	attach(aPintNumber, bPinNumber, encType::full);
}

void ESP32Encoder::setCount(int64_t value) {
	_ENTER_CRITICAL();
	count = value - getCountRaw();
	_EXIT_CRITICAL();
}

int64_t ESP32Encoder::getCountRaw() {
	int16_t c;
	int64_t compensate = 0;
	_ENTER_CRITICAL();
	pcnt_get_counter_value(unit, &c);
	// check if counter overflowed, if so re-read and compensate
	// see https://github.com/espressif/esp-idf/blob/v4.4.1/tools/unit-test-app/components/test_utils/ref_clock_impl_rmt_pcnt.c#L168-L172
	if (PCNT.int_st.val & BIT(unit)) {
        pcnt_get_counter_value(unit, &c);
		if(PCNT.status_unit[unit].COUNTER_H_LIM){
			compensate = r_enc_config.counter_h_lim;
		} else if (PCNT.status_unit[unit].COUNTER_L_LIM) {
			compensate = r_enc_config.counter_l_lim;
		}
	}
	_EXIT_CRITICAL();
	return compensate + c;
}

int64_t ESP32Encoder::getCount() {
	_ENTER_CRITICAL();
	int64_t result = count + getCountRaw();
	_EXIT_CRITICAL();
	return result;
}

int64_t ESP32Encoder::clearCount() {
	_ENTER_CRITICAL();
	count = 0;
	_EXIT_CRITICAL();
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
#else
#warning PCNT not supported on this SoC, this will likely lead to linker errors when using ESP32Encoder
#endif // SOC_PCNT_SUPPORTED
