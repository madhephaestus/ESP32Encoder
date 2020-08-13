# ESP32Encoder

[![Build Status](https://travis-ci.com/madhephaestus/ESP32Encoder.svg?branch=master)](https://travis-ci.com/github/madhephaestus/ESP32Encoder)


ESP32Encoder library that uses the ESP32 pulse counter hardware peripheral:

https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/peripherals/pcnt.html

There is only one interrupt for the peripheral, and that is managed by the library. The user has no interrupt interface, and no interrupts are generated on each pulse. Interrupts arrive when the 16 bit counter buffer overflows, so this library has a tiny interrupt footprint while providing support for up to 10 simultaneous quadrature encoders.

# Documentation by Doxygen

[ESP32Encoder Doxygen](https://madhephaestus.github.io/ESP32Encoder/files.html)


## Pull Downs

The 2 encoder pins have an internal weak pull down. This is to prevent erroneous ticking when disconnected. 
