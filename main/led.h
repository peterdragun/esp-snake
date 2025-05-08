#ifndef LED_H
#define LED_H

#include "driver/rmt_tx.h"
#include "led_strip.h"
#include "sdkconfig.h"


// LED Matrix configuration
#define LED_MATRIX_SIZE CONFIG_SNAKE_LED_MATRIX_SIZE
#define LED_MATRIX_PIN CONFIG_SNAKE_LED_GPIO
#define LED_MATRIX_RMT_CHANNEL RMT_CHANNEL_0



void init_led_matrix(led_strip_handle_t *led_strip);

void draw_pixel(led_strip_handle_t led_strip, int x, int y, uint32_t color);

void clear_matrix(led_strip_handle_t led_strip);

#endif // LED_H
