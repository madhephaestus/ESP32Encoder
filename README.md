# ESP32Encoder

ESP32Encoder library uses the ESP32 pulse counter hardware peripheral:

https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/peripherals/pcnt.html

There is only one interrupt for the peripheral, and that is managed by the library. the user has no interrupt interface, and no interrupts are generated on each pulse. interrupts come when the 16 bit counter buffer overflows, so this library has a tiny interrupt footprint while providing 10 simultaneous quadrature encoders.

## Pull Downs

The 2 encoder pins have an internal weak pull down. This is to prevent erronious ticking when disconnected. 
