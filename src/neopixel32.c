// neopixel32.c
// Copyright (c) 2020 Johannes Berndorfer (berndoJ)

#include "neopixel32.h"
#include <stdlib.h>

static void _NP32_Recalc_DMA_Buf(NP32_Instance_t *handle);

int8_t NP32_Init(NP32_Instance_t *handle)
{
    // Check for LED count.
    if (handle->LED_Count == 0)
        return -1;
    
    // Allocate memory for the LED color buffer.
    handle->LED_Col_Buffer = (NP32_RGB_t *) calloc(handle->LED_Count, sizeof(NP32_RGB_t));
    if (handle->LED_Col_Buffer == NULL)
        return -1;
    // Allocate memory for the DMA buffer.
    handle->_DMA_Buffer = (uint16_t *) calloc((handle->LED_Count * 24) + NP32_WS2812_ZERO_PERIODS, sizeof(uint16_t));
    if (handle->_DMA_Buffer == NULL)
        return -1;

    // Check if DMA call is null.
    if (handle->StartDMA_Call == NULL)
        return -1;

    return 0;
}

void NP32_DeInit(NP32_Instance_t *handle)
{
    // Free the LED color buffer and the DMA buffer.
    free(handle->LED_Col_Buffer);
    free(handle->_DMA_Buffer);

    // Set the count of LEDs to 0.
    handle->LED_Count = 0;
}

int8_t NP32_Update(NP32_Instance_t *handle)
{
    // Check for initialised handle.
    if (handle->LED_Col_Buffer == NULL || handle->_DMA_Buffer == NULL)
        return -1;
    
    // Wait for the last update to complete.
    while (handle->DMA_Busy_Flag);

    // Recalculate the DMA-buffer.
    _NP32_Recalc_DMA_Buf(handle);

    // Start the DMA stream to the PWM peripheral.
    handle->DMA_Busy_Flag = 1U;
    handle->StartDMA_Call(handle->_DMA_Buffer, (handle->LED_Count * 24) + NP32_WS2812_ZERO_PERIODS);

    return 0;
}

void NP32_DMAComplete_Callback(NP32_Instance_t *handle)
{
    if (handle->DMA_Busy_Flag)
        handle->DMA_Busy_Flag = 0;
}

void NP32_HSV_To_RGB(NP32_HSV_t hsv, NP32_RGB_t *rgb)
{
    uint8_t d;

    // Check for overflow values.
    if (hsv.H > 359) hsv.H = 359;
    if (hsv.S > 100) hsv.S = 100;
    if (hsv.V > 100) hsv.V = 100;

    // Convert H value.
    if (hsv.H < 61)
    {
        rgb->R = 255;
        rgb->G = (425 * hsv.H) / 100;
        rgb->B = 0;
    }
    else if (hsv.H < 121)
    {
        rgb->R = 255- ((425 * (hsv.H - 60)) / 100);
        rgb->G = 255;
        rgb->B = 0;
    }
    else if (hsv.H < 181)
    {
        rgb->R = 0;
        rgb->G = 255;
        rgb->B = (425 * (hsv.H -120)) / 100;
    }
    else if (hsv.H < 241)
    {
        rgb->R = 0;
        rgb->G = 255 - ((425 * (hsv.H - 180)) / 100);
        rgb->B = 255;
    }
    else if (hsv.H < 301)
    {
        rgb->R = (425 * (hsv.H - 240)) / 100;
        rgb->G = 0;
        rgb->B = 255;
    }
    else
    {
        rgb->R = 255;
        rgb->G = 0;
        rgb->B = 255 - ((425 * (hsv.H - 300)) / 100);
    }

    // Convert S value.
    hsv.S = 100 - hsv.S;
    // R
    d = ((255 - rgb->R) * hsv.S) / 100;
    rgb->R += d;
    // G
    d = ((255 - rgb->G) * hsv.S) / 100;
    rgb->G += d;
    // B
    d = ((255 - rgb->B) * hsv.S) / 100;
    rgb->B += d;

    // Convert V value.
    rgb->R = (rgb->R * hsv.V) / 100;
    rgb->G = (rgb->G * hsv.V) / 100;
    rgb->B = (rgb->B * hsv.V) / 100;
}

/*--------------------------------------------------------------------------------------------------------------------*/

int8_t NP32_SetLED_RGB(NP32_Instance_t *handle, uint16_t led_index, NP32_RGB_t rgb)
{
    if (led_index >= handle->LED_Count)
        return -1;
    
    handle->LED_Col_Buffer[led_index] = rgb;

    return 0;
}

int8_t NP32_SetLED_HSV(NP32_Instance_t *handle, uint16_t led_index, NP32_HSV_t hsv)
{
    NP32_RGB_t rgb;
    NP32_HSV_To_RGB(hsv, &rgb);
    return NP32_SetLED_RGB(handle, led_index, rgb);
}

int8_t NP32_SetAllLEDs_RGB(NP32_Instance_t *handle, NP32_RGB_t rgb)
{
    uint16_t i;

    for (i = 0; i < handle->LED_Count; i++)
    {
        handle->LED_Col_Buffer[i] = rgb;
    }

    return 0;
}

int8_t NP32_SetAllLEDs_HSV(NP32_Instance_t *handle, NP32_HSV_t hsv)
{
    NP32_RGB_t rgb;
    NP32_HSV_To_RGB(hsv, &rgb);
    return NP32_SetAllLEDs_RGB(handle, rgb);
}

int8_t NP32_SetLEDSpan_RGB(NP32_Instance_t *handle, uint16_t lower_bound, uint16_t higher_bound, NP32_RGB_t rgb)
{
    uint16_t i;

    if (lower_bound >= handle->LED_Count || higher_bound >= handle->LED_Count || lower_bound > higher_bound)
        return -1;
    
    for (i = lower_bound; i <= higher_bound; i++)
    {
        handle->LED_Col_Buffer[i] = rgb;
    }

    return 0;
}

int8_t NP32_SetLEDSpan_HSV(NP32_Instance_t *handle, uint16_t lower_bound, uint16_t higher_bound, NP32_HSV_t hsv)
{
    NP32_RGB_t rgb;
    NP32_HSV_To_RGB(hsv, &rgb);
    return NP32_SetLEDSpan_RGB(handle, lower_bound, higher_bound, rgb);
}

int8_t NP32_ClearAllLEDs(NP32_Instance_t *handle)
{
    return NP32_SetAllLEDs_RGB(handle, NP32_COL_BLACK);
}

int8_t NP32_ShiftLeft(NP32_Instance_t *handle, uint16_t shift_amount)
{
    uint16_t i, s;

    for (i = 0; i < handle->LED_Count; i++)
    {
        s = shift_amount + i;
        if (s >= handle->LED_Count)
            handle->LED_Col_Buffer[i] = NP32_COL_BLACK;
        else
            handle->LED_Col_Buffer[i] = handle->LED_Col_Buffer[s];    
    }

    return 0;
}

int8_t NP32_ShiftRight(NP32_Instance_t *handle, uint16_t shift_amount)
{
    uint16_t i;

    for (i = handle->LED_Count - 1; i >= 0; i--)
    {
        if (shift_amount > i)
            handle->LED_Col_Buffer[i] = NP32_COL_BLACK;
        else
            handle->LED_Col_Buffer[i] = handle->LED_Col_Buffer[i - shift_amount];    
    }

    return 0;
}

int8_t NP32_RotateLeft(NP32_Instance_t *handle, uint16_t rotate_amount)
{
    NP32_RGB_t *buf;
    uint16_t i, s;

    buf = (NP32_RGB_t *) calloc(rotate_amount, sizeof(NP32_RGB_t));
    if (buf == NULL)
        return -1;

    for (i = 0; i < handle->LED_Count; i++)
    {
        s = rotate_amount + i;

        if (i < rotate_amount)
            buf[i] = handle->LED_Col_Buffer[i];

        if (s >= handle->LED_Count)
            handle->LED_Col_Buffer[i] = buf[s - handle->LED_Count];
        else
            handle->LED_Col_Buffer[i] = handle->LED_Col_Buffer[s];    
    }

    free(buf);

    return 0;
}

int8_t NP32_RotateRight(NP32_Instance_t *handle, uint16_t rotate_amount)
{
    NP32_RGB_t *buf;
    uint16_t i;

    buf = (NP32_RGB_t *) calloc(rotate_amount, sizeof(NP32_RGB_t));
    if (buf == NULL)
        return -1;

    for (i = handle->LED_Count - 1; i >= 0; i--)
    {
        if (i >= handle->LED_Count - rotate_amount)
            buf[i - (handle->LED_Count - rotate_amount)] = handle->LED_Col_Buffer[i];

        if (rotate_amount > i)
            handle->LED_Col_Buffer[i] = buf[i];
        else
            handle->LED_Col_Buffer[i] = handle->LED_Col_Buffer[i - rotate_amount];    
    }

    free(buf);

    return 0;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void _NP32_Recalc_DMA_Buf(NP32_Instance_t *handle)
{
    NP32_RGB_t curr_col;
    uint16_t i, i_dma = 0;
    uint8_t b;

    if (handle->LED_Disable_Flag == 1U)
    {
        for (i_dma = 0; i_dma < (handle->LED_Count * 24) + NP32_WS2812_ZERO_PERIODS; i_dma++)
        {
            handle->_DMA_Buffer[i_dma] = 0x00;
        }
    }
    else
    {
        // Fill the DMA buffer with color values.
        for (i = 0; i < handle->LED_Count; i++)
        {
            // Get the current LED color.
            curr_col = handle->LED_Col_Buffer[i];

            // Fill the DMA buffer.
            i_dma = i * 24;
            // Bits G7 to G0.
            b = 8;
            do
            {
                handle->_DMA_Buffer[i_dma++] = NP32_RESOLVE_BIT_TIME(curr_col.G, --b);
            }
            while (b > 0);
            // Bits R7 to R0.
            b = 8;
            do
            {
                handle->_DMA_Buffer[i_dma++] = NP32_RESOLVE_BIT_TIME(curr_col.R, --b);
            }
            while (b > 0);
            // Bits B7 to B0.
            b = 8;
            do
            {
                handle->_DMA_Buffer[i_dma++] = NP32_RESOLVE_BIT_TIME(curr_col.B, --b);
            }
            while (b > 0);
        }

        // Append 0 values for the reset time.
        for (i = 0; i < NP32_WS2812_ZERO_PERIODS; i++)
        {
            handle->_DMA_Buffer[i_dma++] = 0x00;
        }
    }

    return;
}