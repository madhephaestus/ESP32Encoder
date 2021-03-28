# ESP32Encoder

[![Build Status](https://travis-ci.com/madhephaestus/ESP32Encoder.svg?branch=master)](https://travis-ci.com/github/madhephaestus/ESP32Encoder)


ESP32Encoder library that uses the ESP32 pulse counter hardware peripheral:

https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/peripherals/pcnt.html

There is only one interrupt for the peripheral, and that is managed by the library. The user has no interrupt interface, and no interrupts are generated on each pulse. Interrupts arrive when the 16 bit counter buffer overflows, so this library has a tiny interrupt footprint while providing support for up to 10 simultaneous quadrature encoders.

# Documentation by Doxygen

[ESP32Encoder Doxygen](https://madhephaestus.github.io/ESP32Encoder/classESP32Encoder.html)


## Pull Downs

The 2 encoder pins have an internal weak pull down. This is to prevent erroneous ticking when disconnected. 

# A note on KY-040 and similar

The "switch style" encoder wheels used by breakout modules such as:

https://usa.banggood.com/KY-040-360-Degrees-Rotary-Encoder-Module-with-1516_5mm-Potentiometer-Rotary-Knob-Cap-for-Brick-Sensor-Switch-p-1677837.html

need electrical debouncing in the range of 0.1 - 0.22 uf per encoder line to ground. This device bounces much more than the PCNT hardware modules debouncing time limits.  Be sure to [setFilter()](https://madhephaestus.github.io/ESP32Encoder/classESP32Encoder.html#ae3cecb7d572685b3195f8a13409b3390) to 1024 as well after attaching in order to get the maximum hardware debouncing. 
