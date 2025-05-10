#include "numbers.h"

#include <stddef.h>

// Function to draw a two-digit number on the 8x8 LED matrix
void draw_number(led_strip_handle_t led_strip, int number, uint32_t color) {
    if (number < 0 || number > 99) {
        return; // Invalid number
    }

    int tens = number / 10; // Extract the tens digit
    int ones = number % 10; // Extract the ones digit

    const uint8_t *bitmap_tens = (number >= 10) ? number_font_4x8[tens] : NULL; // Get tens bitmap if applicable
    const uint8_t *bitmap_ones = number_font_4x8[ones]; // Get ones bitmap

    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            if (number < 10) {
                // Center the single digit on the matrix
                if (x >= 2 && x < 6 && (bitmap_ones[y] & (1 << (5 - x)))) {
                    draw_pixel(led_strip, x, y, color); // Draw the single digit centered
                } else {
                    draw_pixel(led_strip, x, y, 0xFFFFFF); // Clear the pixel
                }
            } else {
                // Draw two digits side by side
                if (x < 4 && bitmap_tens && (bitmap_tens[y] & (1 << (3 - x)))) {
                    draw_pixel(led_strip, x, y, color); // Draw the tens digit on the left half
                } else if (x >= 4 && (bitmap_ones[y] & (1 << (7 - x)))) {
                    draw_pixel(led_strip, x, y, color); // Draw the ones digit on the right half
                } else {
                    draw_pixel(led_strip, x, y, 0xFFFFFF); // Clear the pixel
                }
            }
        }
    }
}
