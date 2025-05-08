#ifndef NUMBERS_H
#define NUMBERS_H

#include <stdint.h>

#include "led.h"


// Define the 4x8 font for numbers 0-9
static const uint8_t number_font_4x8[10][8] = {
    {0b0110, 0b1001, 0b1001, 0b1001, 0b1001, 0b1001, 0b0110, 0b0000}, // 0
    {0b0010, 0b0110, 0b0010, 0b0010, 0b0010, 0b0010, 0b0111, 0b0000}, // 1
    {0b0110, 0b1001, 0b0001, 0b0010, 0b0100, 0b1000, 0b1111, 0b0000}, // 2
    {0b0110, 0b1001, 0b0001, 0b0010, 0b0001, 0b1001, 0b0110, 0b0000}, // 3
    {0b0001, 0b0011, 0b0101, 0b1001, 0b1111, 0b0001, 0b0001, 0b0000}, // 4
    {0b1111, 0b1000, 0b1110, 0b0001, 0b0001, 0b1001, 0b0110, 0b0000}, // 5
    {0b0110, 0b1001, 0b1000, 0b1110, 0b1001, 0b1001, 0b0110, 0b0000}, // 6
    {0b1111, 0b0001, 0b0010, 0b0010, 0b0100, 0b0100, 0b0100, 0b0000}, // 7
    {0b0110, 0b1001, 0b1001, 0b0110, 0b1001, 0b1001, 0b0110, 0b0000}, // 8
    {0b0110, 0b1001, 0b1001, 0b0111, 0b0001, 0b1001, 0b0110, 0b0000}  // 9
};

void draw_number(led_strip_handle_t led_strip, int number, uint32_t color);

#endif // NUMBERS_H
