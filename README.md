# libneopixel32

A library for Neopixel / WS2812 LEDs for STM32-microcontroller devices. This library is designed for STM32s, but could be altered in order to support other microcontroller devices (running an ARM-Cortex processor). This would require changing some definitions and possibly the callback functions to the HAL.

## Implementation

The library requires a call function to start a DMA stream from a buffer (the pointer and length is given to the call function) to the PWM peripheral of the procesor. The library also provides a callback function for signaling the library, that the DMA stream has completed.

## Building the Library

The Makefile provided by this repository targets a ARM Cortex-M3 STM32 processor. This can be changed, there are just a few minor modifications needed in the Makefile for other STM32 processors.

The compilation needs GNU-Make and the ARM embedded toolchain. (`make`, `arm-none-eabi-gcc`, ...)

If you are using Windows, I prefer using Ubuntu running on WSL to get a Linux build environment. (My setup is Windows, WSL, Ubuntu for Windows, Microsoft Terminal and the toolchain needed to compile)

If you have any questions, feel free to contact me via the repository issues or via email (See my profile for the address).

The latest build of the library can be found in the repository in the directory `/bin`. The latest compiled static library file can be found under `/bin/libneopixel32.a`.

## Future Updates

I plan to update this library occasionally and fixing bugs brought to me via the Issues tab on GitHub. All planned future features are layed out in the Projects tab of this repository. If you wish to add a new feature, just create an issue on this repo or create a pull request.

## License

This library is MIT-licensed, so feel free to use it.
