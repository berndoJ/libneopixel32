# libneopixel32

A library for Neopixel / WS2812 LEDs for STM32-microcontroller devices. This library is designed for STM32s, but could be altered in order to support other microcontroller devices (running an ARM-Cortex processor). This would require changing some definitions and possibly the callback functions to the HAL.

The library requires a call function to start a DMA stream from a buffer (the pointer and length is given to the call function) to the PWM peripheral of the procesor.
