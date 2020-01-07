/*******************************************************************************
* \file neopixel32.h
* \date 05.01.2020
********************************************************************************
* \author Johannes Berndorfer (berndoJ)
* \copyright Copyright (c) 2020 Johannes Berndorfer (berndoJ)
********************************************************************************
* \brief Main header file for libneopixel32 - a STM32 HAL compatible neopixel /
*        WS2812 LED driver library.
*
* \paragraph dep_par Dependencies
*        The following dependencies are needed for this library:
*          - libc *(stdint.h)*
*******************************************************************************/

#if !defined(__NEOPIXEL32_H)
#define __NEOPIXEL32_H

#include <stdint.h>

/**
 * \brief The timer count period at a CPU clock frequency of 72MHz.
 * \note 1 clock cycle @ 72MHz = 13.88ns; 1 clock cycle of 1 bit sent to the neopixel / WS2812 LED = 1.3μs -> 93.6 *
 *       13.88ns = 1300ns. The cycle count gets rounded to 94. As the timer starts counting @0 -> 93.
 */
#define NP32_WS2812_TIM_PERIOD          93U

/**
 * \brief The number of timer counts to reach the high time of a 0 bit.
 * \note T0H = 400ns (of 1300ns) -> (4/13) * 94 = 28.92 -> rounded to 29. As the timer starts counting @0 -> 28.
 */
#define NP32_WS2812_0_TIME              28U

/**
 * \brief The number of timer counts to reach the high time of a 1 bit.
 * \note T0H = 700ns (of 1300ns) -> (7/13) * 94 = 50.61 -> rounded to 51. As the timer starts counting @0 -> 50.
 */
#define NP32_WS2812_1_TIME              50U 

/**
 * \brief The amount of zero-periods sent after the LED data as a reset command.
 */
#define NP32_WS2812_ZERO_PERIODS        48U

/**
 * \brief This macro gets the given bit of the given value (0 = LSB, 7 = MSB) and returns \ref NP32_WS2812_1_TIME if the
 *        bit is 1 and \ref NP32_WS2812_0_TIME if the bit is 0.
 */
#define NP32_RESOLVE_BIT_TIME(val,bit)  (((val >> bit) & 0x01) ? NP32_WS2812_1_TIME : NP32_WS2812_0_TIME)

#define NP32_COL_BLACK                  ((NP32_RGB_t){ 0x00, 0x00, 0x00 })

/**
 * \brief Data structure that represents a RGB-value of a neopixel LED.
 */
struct __NP32_RGB
{  
    uint8_t R; /**< The red component of represented colour. */
    uint8_t G; /**< The green component of represented colour. */
    uint8_t B; /**< The blue component of represented colour. */
};

/**
 * \brief Data structure that represents a HSV-value of a neopixel LED.
 */
struct __NP32_HSV
{
    uint16_t H; /**< The hue (colour) of the represented colour. */
    uint8_t S;  /**< The saturation of the represented colour. */
    uint8_t V;  /**< The value (brightness) of the represented colour. */
};

/**
 * \brief Represents an instance of neopixel / WS2812 LEDs controlled by a single PWM pin on the processor. This struct
 *        enables the feature of having multiple pins controlling multiple arrays of neopixel / WS2812 LEDs on a single
 *        processor running this library.
 */
struct __NP32_Instance
{
    uint16_t LED_Count; /**< The count of LEDs in the array this instance represents. */

    struct __NP32_RGB *LED_Col_Buffer; /**< The pointer to the LED colour buffer, which holds the values of all LEDs in
                                              the LED array this instance represents.*/

    uint8_t LED_Disable_Flag; /**< This flag enables (if set to 1) the LED disable mode. In this mode, the colour buffer
                                   is still present (and can still be manipulated by the program), but the LEDs all get
                                   set to black. This enables a global shutoff of the LEDs without emptying the colour
                                   buffer. */

    uint16_t *_DMA_Buffer; /**< The pointer to the DMA half-word buffer, which holds the raw PWM information to output
                                to the LED data in pin of the LED array this instance represents. */

    volatile uint8_t DMA_Busy_Flag; /**< Busy flag for the DMA. 0 = Ready, 1 = Busy. */

    int8_t (* StartDMA_Call)(uint16_t *buf, uint16_t len); /**< Function delegate for starting the DMA stream from the
                                                                given buffer with the given length. */
};

typedef struct __NP32_RGB        NP32_RGB_t;
typedef struct __NP32_HSV        NP32_HSV_t;
typedef struct __NP32_Instance   NP32_Instance_t;

/**
 * \brief Initialises the provided \ref NP32_Instance_t and allocates the needed memory for the instance. This function
 *        has to be called before any other function of this library gets called with the given instance. It is crucial,
 *        that the corresponding HAL code for this instance is initialised before calling this init function and is
 *        fully operational at the time of calling this function.
 * \param handle The instance to initialise.
 * \return The status. If -1, then an error occurred while executing this function. If 0, the function terminated
 *         successfully.
 */
int8_t NP32_Init(NP32_Instance_t *handle);

/**
 * \brief Deinitialises the given NP32 instance and frees up the memory allocated by its buffers.
 * \param handle The instance to de-init.
 */
void NP32_DeInit(NP32_Instance_t *handle);

/**
 * \brief This function updates the neopixel / WS2812 LED array, specified by the NP32 instance given to this function,
 *        to the current state of the LED colour buffer. This function recalculates the DMA buffer from the current LED
 *        colour buffer and intiates a DMA write to the corresponding PWM peripheral / GPIO pin.
 * \param handle The instance which represents the neopixel / WS2812 LED array to update.
 * \return The status. If -1, then an error occurred while executing this function. If 0, the function terminated
 *         successfully.
 */
int8_t NP32_Update(NP32_Instance_t *handle);

/**
 * \brief Callback function for letting the library know, that the DMA stream of the previous DMA buffer is complete.
 *        This function should normally be called from the DMA-complete interrupt of the underlying HAL.
 * \param handle The instance, of which the DMA stream completed.
 */
void NP32_DMAComplete_Callback(NP32_Instance_t *handle);

/**
 * \brief Function to convert from a HSV colour value to a RGB colour value.
 * \param hsv The HSV value to convert.
 * \param rgb The pointer to the RGB value to write the converted value into.
 * \note Code based on www.ulrichradig.de
 */
void NP32_HSV_To_RGB(NP32_HSV_t hsv, NP32_RGB_t *rgb);

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Sets the colour of the specified LED.
 * \param handle The instance the LED belongs to.
 * \param led_index The index of the LED in the data chain.
 * \param rgb The new colour of the LED in RGB.
 * \return The status. If -1, then an error occurred while executing this function. If 0, the function terminated
 *         successfully.
 */
int8_t NP32_SetLED_RGB(NP32_Instance_t *handle, uint16_t led_index, NP32_RGB_t rgb);

/**
 * \brief Sets the colour of the specified LED.
 * \param handle The instance the LED belongs to.
 * \param led_index The index of the LED in the data chain.
 * \param hsv The new colour of the LED in HSV.
 * \return The status. If -1, then an error occurred while executing this function. If 0, the function terminated
 *         successfully.
 */
int8_t NP32_SetLED_HSV(NP32_Instance_t *handle, uint16_t led_index, NP32_HSV_t hsv);

/**
 * \brief Sets all LEDs of the given instance to the specified colour.
 * \param handle The instance of the LEDs to set the colour of.
 * \param rgb The new colour of all LEDs (of the given instance) in RGB.
 * \return The status. If -1, then an error occurred while executing this function. If 0, the function terminated
 *         successfully.
 */
int8_t NP32_SetAllLEDs_RGB(NP32_Instance_t *handle, NP32_RGB_t rgb);

/**
 * \brief Sets all LEDs of the given instance to the specified colour.
 * \param handle The instance of the LEDs to set the colour of.
 * \param hsv The new colour of all LEDs (of the given instance) in HSV.
 * \return The status. If -1, then an error occurred while executing this function. If 0, the function terminated
 *         successfully.
 */
int8_t NP32_SetAllLEDs_HSV(NP32_Instance_t *handle, NP32_HSV_t hsv);

/**
 * \brief Sets a span of LEDs of the given instance to the specified colour. This is like setting a specified span of
 *        elements of an array to the same value.
 * \param handle The instance of the LEDs.
 * \param lower_bound The lower bound index of the array of LEDs of which the colour should be set.
 * \param higher_bound The higher bound index of the array of LEDs of which the colour should be set. It is necessary
 *                     that this parameter is greater or equal than the lower_bound parameter.
 * \param rgb The new colour of the LEDs in the specified span in RGB.
 * \return The status. If -1, then an error occurred while executing this function. If 0, the function terminated
 *         successfully.
 */
int8_t NP32_SetLEDSpan_RGB(NP32_Instance_t *handle, uint16_t lower_bound, uint16_t higher_bound, NP32_RGB_t rgb);

/**
 * \brief Sets a span of LEDs of the given instance to the specified colour. This is like setting a specified span of
 *        elements of an array to the same value.
 * \param handle The instance of the LEDs.
 * \param lower_bound The lower bound index of the array of LEDs of which the colour should be set.
 * \param higher_bound The higher bound index of the array of LEDs of which the colour should be set. It is necessary
 *                     that this parameter is greater or equal than the lower_bound parameter.
 * \param hsv The new colour of the LEDs in the specified span in HSV.
 * \return The status. If -1, then an error occurred while executing this function. If 0, the function terminated
 *         successfully.
 */
int8_t NP32_SetLEDSpan_HSV(NP32_Instance_t *handle, uint16_t lower_bound, uint16_t higher_bound, NP32_HSV_t hsv);

/**
 * \brief Sets the colour of all LEDs of the given instance to black (RGB 0,0,0).
 * \param handle The instance of the LEDs to clear.
 * \return The status. If -1, then an error occurred while executing this function. If 0, the function terminated
 *         successfully.
 */
int8_t NP32_ClearAllLEDs(NP32_Instance_t *handle);

/**
 * \brief Shifts the whole LED colour buffer to the left. (Left means that the colour of LED x will be the previous
 *        colour of the LED x + 1). The gap, which is created by shifting all LEDs to the left will be filled with the
 *        colour black.
 * \param handle The instance of the LEDs to shift.
 * \param shift_amount The amount of spots to shift the LEDs.
 * \return The status. If -1, then an error occurred while executing this function. If 0, the function terminated
 *         successfully.
 */
int8_t NP32_ShiftLeft(NP32_Instance_t *handle, uint16_t shift_amount);

/**
 * \brief Shifts the whole LED colour buffer to the right. (Right means that the colour of LED x will be the previous
 *        colour of the LED x - 1). The gap, which is created by shifting all LEDs to the right will be filled with the
 *        colour black.
 * \param handle The instance of the LEDs to shift.
 * \param shift_amount The amount of spots to shift the LEDs.
 * \return The status. If -1, then an error occurred while executing this function. If 0, the function terminated
 *         successfully.
 */
int8_t NP32_ShiftRight(NP32_Instance_t *handle, uint16_t shift_amount);

/**
 * \brief Rotates the whole LED colour buffer to the left. (Left means that the colour of LED x will be the previous
 *        colour of the LED x + 1). This function places the colours, which get shiftet out of bounds, at the end of the
 *        LED array. Therefore no colours are lost and there is no need for filling in gaps with colour black. This can
 *        be useful if the LEDs are arranged in a circle, because a rotate of the LED array will also rotate the colours
 *        on the circle of LEDs.
 * \param handle The instance of the LEDs to rotate.
 * \param rotate_amount The amount of spots to roteate the LEDs.
 * \return The status. If -1, then an error occurred while executing this function. If 0, the function terminated
 *         successfully.
 */
int8_t NP32_RotateLeft(NP32_Instance_t *handle, uint16_t rotate_amount);

/**
 * \brief Rotates the whole LED colour buffer to the left. (Left means that the colour of LED x will be the previous
 *        colour of the LED x + 1). This function places the colours, which get shiftet out of bounds, at the end of the
 *        LED array. Therefore no colours are lost and there is no need for filling in gaps with colour black. This can
 *        be useful if the LEDs are arranged in a circle, because a rotate of the LED array will also rotate the colours
 *        on the circle of LEDs.
 * \param handle The instance of the LEDs to rotate.
 * \param rotate_amount The amount of spots to roteate the LEDs.
 * \return The status. If -1, then an error occurred while executing this function. If 0, the function terminated
 *         successfully.
 */
int8_t NP32_RotateRight(NP32_Instance_t *handle, uint16_t rotate_amount);


#endif // __NEOPIXEL32_H