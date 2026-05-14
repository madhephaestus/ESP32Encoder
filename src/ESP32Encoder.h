#pragma once
#include <driver/gpio.h>
#include <driver/pcnt.h>
#include <stdint.h>

#ifdef ESP32ENCODER_USE_INT32
typedef int32_t enc_count_t;
#else
typedef int64_t enc_count_t;
#endif

#ifndef ARDUINO
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/portable.h>
#include <freertos/semphr.h>
#endif

#define MAX_ESP32_ENCODERS PCNT_UNIT_MAX
#define 	_INT16_MAX 32766
#define  	_INT16_MIN -32766
#define ISR_CORE_USE_DEFAULT (0xffffffff)

enum class encType {
	single,
	half,
	full
};

enum class puType {
	up,
	down,
	none
};

class ESP32Encoder;

typedef void (*enc_isr_cb_t)(void*);

class ESP32Encoder {
public:
	/**
	 * @brief Construct a new ESP32Encoder object
	 *
	 * @param always_interrupt set to true to enable interrupt on every encoder pulse, otherwise false
	 * @param enc_isr_cb callback executed on every encoder ISR, gets a pointer to
	 * 	the ESP32Encoder instance as an argument, no effect if always_interrupt is
	 * 	false
	 */
	ESP32Encoder(bool always_interrupt=false, enc_isr_cb_t enc_isr_cb=nullptr, void* enc_isr_cb_data=nullptr);
	~ESP32Encoder();
	void attachHalfQuad(int aPintNumber, int bPinNumber);
	void attachFullQuad(int aPintNumber, int bPinNumber);
	void attachSingleEdge(int aPintNumber, int bPinNumber);
	enc_count_t getCount();
	enc_count_t clearCount();
	enc_count_t pauseCount();
	enc_count_t resumeCount();
	void detach();
	[[deprecated("Replaced by detach")]] void detatch();
	bool isAttached(){return attached;}
	void setCount(enc_count_t value);
	void setFilter(uint16_t value);
	static ESP32Encoder *encoders[MAX_ESP32_ENCODERS];
	bool always_interrupt;
	gpio_num_t aPinNumber;
	gpio_num_t bPinNumber;
	pcnt_unit_t unit;
	int countsMode = 2;
	volatile enc_count_t count=0;
	pcnt_config_t r_enc_config;
	static puType useInternalWeakPullResistors;
	static uint32_t isrServiceCpuCore;
	enc_isr_cb_t _enc_isr_cb;
	void* _enc_isr_cb_data;

private:
	static bool attachedInterrupt;
	void attach(int aPintNumber, int bPinNumber, encType et);
	enc_count_t getCountRaw();
	bool attached;
  bool direction;
  bool working;
};

//Added by Sloeber
#pragma once
