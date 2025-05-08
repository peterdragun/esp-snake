#include "led.h"

void init_led_matrix(led_strip_handle_t *led_strip) {
    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_MATRIX_PIN,
        .max_leds = LED_MATRIX_SIZE,
    };
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, led_strip));
    led_strip_clear(*led_strip);
}

// Function to draw a pixel on the LED matrix
void draw_pixel(led_strip_handle_t led_strip, int x, int y, uint32_t color) {
    if (x >= 0 && x < 8 && y >= 0 && y < 8) {
        int index = y * 8 + x;
        led_strip_set_pixel(led_strip, index, (color >> 16) & 0x1F, (color >> 8) & 0x1F, color & 0x1F);
    }
}

// Function to clear the LED matrix
void clear_matrix(led_strip_handle_t led_strip) {
    led_strip_clear(led_strip);
}
