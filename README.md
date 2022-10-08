# ESP32Encoder

[![Build Status](https://travis-ci.com/madhephaestus/ESP32Encoder.svg?branch=master)](https://travis-ci.com/github/madhephaestus/ESP32Encoder)


ESP32Encoder library that uses the ESP32 pulse counter hardware peripheral:

https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/peripherals/pcnt.html

There is only one interrupt for the peripheral, and that is managed by the library. The user has no interrupt interface, and no interrupts are generated on each pulse. Interrupts arrive when the 16 bit counter buffer overflows, so this library has a tiny interrupt footprint while providing support for up to 8 simultaneous quadrature encoders.

This hardware peripheral supports only 8 encoders. 

# Support

ESP32 and ESP32c2 are supported. 

ESP32c3 does not have pulse counter hardware. 

# Documentation by Doxygen

[ESP32Encoder Doxygen](https://madhephaestus.github.io/ESP32Encoder/classESP32Encoder.html)

# Quadrature Explanation

For information on the type of encoder this library is for, see: https://en.wikipedia.org/wiki/Incremental_encoder

The modes of reading encoders in this library are full and half quadrature, and single edge count mode. 

![Image](https://upload.wikimedia.org/wikipedia/commons/1/1e/Incremental_directional_encoder.gif)

Full performs a count increment on all 4 edges, half on the rising and falling of a single channel, and single counts just the rising edge of the A channel. 



## Pull Downs/Ups

To specify the weak pull resistor set the value [useInternalWeakPullResistors](https://madhephaestus.github.io/ESP32Encoder/classESP32Encoder.html#a53dc40c9de240e90a55b427b32da451f) with the enum types [UP, DOWN, or NONE](https://madhephaestus.github.io/ESP32Encoder/ESP32Encoder_8h.html#adca399663765c125d26e6f2896b5b349)

## ISR service CPU core
To specify the CPU core for the PCNT ISR service set the value [isrServiceCpuCore](https://madhephaestus.github.io/ESP32Encoder/classESP32Encoder.html#a445e515b31ade19658075aa7417086bb) to the desired core number. This can be usefull to prevent concurrency problems where the total count might not be updated correctly yet while it is read.

# A note on KY-040 and similar

The "switch style" encoder wheels used by breakout modules such as:

https://usa.banggood.com/KY-040-360-Degrees-Rotary-Encoder-Module-with-1516_5mm-Potentiometer-Rotary-Knob-Cap-for-Brick-Sensor-Switch-p-1677837.html

need electrical debouncing in the range of 0.1 - 2 uf per encoder line to ground. This device bounces much more than the PCNT hardware modules debouncing time limits.  Be sure to [setFilter()](https://madhephaestus.github.io/ESP32Encoder/classESP32Encoder.html#ae3cecb7d572685b3195f8a13409b3390) to 1023 as well after attaching in order to get the maximum hardware debouncing. 
