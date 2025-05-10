#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_random.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"

#include "led.h"
#include "numbers.h"

static led_strip_handle_t led_strip;

// Button GPIO pins
#define BUTTON_UP CONFIG_BUTTON_UP
#define BUTTON_DOWN CONFIG_BUTTON_DOWN
#define BUTTON_LEFT CONFIG_BUTTON_LEFT
#define BUTTON_RIGHT CONFIG_BUTTON_RIGHT
#define BUTTON_RESET CONFIG_BUTTON_RESET

#ifdef CONFIG_WALLS_ENABLED
    #define WALLS_ENABLED 1
#else
    #define WALLS_ENABLED 0
#endif

// Snake game configuration
#define SNAKE_MAX_LENGTH CONFIG_SNAKE_LED_MATRIX_SIZE
#define GAME_SPEED_MS CONFIG_SNAKE_SPEED

void save_high_score(uint32_t high_score);

typedef struct {
    int x;
    int y;
} Point;

typedef enum {
    STATE_INIT,
    STATE_PLAYING,
    STATE_GAME_OVER,
    STATE_END
} State;

static Point snake[SNAKE_MAX_LENGTH];
static int snake_length = 3;
static Point food;
static Point direction = {0, 1}; // Initial direction: right
static State state = STATE_INIT;
static uint32_t high_score = 0;

// Function to initialize buttons
void init_buttons() {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BUTTON_UP) | (1ULL << BUTTON_DOWN) | (1ULL << BUTTON_LEFT) | (1ULL << BUTTON_RIGHT) | (1ULL << BUTTON_RESET),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
}

// Function to spawn food at a random location
void spawn_food() {
    food.x = esp_random() % 8;
    food.y = esp_random() % 8;

    // Check if the food is on the snake
    for (int i = 0; i < snake_length; i++) {
        if (food.x == snake[i].x && food.y == snake[i].y) {
            spawn_food();
        }
    }
}

void init_snake() {
    snake[0] = (Point){4, 4};
    snake[1] = (Point){3, 4};
    snake[2] = (Point){2, 4};
    snake_length = 3;
    direction = (Point){0, 0};
    spawn_food();
}

// Function to check button inputs and update direction
void update_direction() {
    bool start = false;
    if (gpio_get_level(BUTTON_UP) == 0 && direction.y == 0) {
        direction.x = 0;
        direction.y = -1;
        start = true;
    } else if (gpio_get_level(BUTTON_DOWN) == 0 && direction.y == 0) {
        direction.x = 0;
        direction.y = 1;
        start = true;
    } else if (gpio_get_level(BUTTON_LEFT) == 0 && direction.x == 0) {
        direction.x = -1;
        direction.y = 0;
        start = true;
    } else if (gpio_get_level(BUTTON_RIGHT) == 0 && direction.x == 0) {
        direction.x = 1;
        direction.y = 0;
        start = true;
    } else if (gpio_get_level(BUTTON_RESET) == 0) {
        // Reset the game
        init_snake();
        state = STATE_INIT;
        ESP_LOGI("snake", "Reset pressed");
    }
    if (state == STATE_INIT && start) {
        state = STATE_PLAYING;
        if (direction.x == -1 && direction.y == 0) {
            // If the snake is moving left, set the direction to right on first move
            direction = (Point){1, 0};
        }
        ESP_LOGI("snake", "Game started");
    }
}

// Function to update the snake's position
void update_snake() {
    if (state != STATE_PLAYING) return;

    // Move the snake
    for (int i = snake_length - 1; i > 0; i--) {
        snake[i] = snake[i - 1];
    }
    snake[0].x += direction.x;
    snake[0].y += direction.y;

    // Check for collisions
    if (WALLS_ENABLED) {
        if (snake[0].x < 0 || snake[0].x >= 8 || snake[0].y < 0 || snake[0].y >= 8) {
            state = STATE_GAME_OVER;
            return;
        }
    }else{
        // Handle negative numbers explicitly
        snake[0].x = (snake[0].x + 8) % 8;
        snake[0].y = (snake[0].y + 8) % 8;
    }
    for (int i = 1; i < snake_length; i++) {
        if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
            state = STATE_GAME_OVER;
            return;
        }
    }

    // Check if the snake eats the food
    if (snake[0].x == food.x && snake[0].y == food.y) {
        if (snake_length < SNAKE_MAX_LENGTH) {
            snake[snake_length].x = snake[snake_length - 1].x;
            snake[snake_length].y = snake[snake_length - 1].y;
            snake_length++;
        }
        spawn_food();
    }
}

// Function to render the game
void render_game() {
    if (state == STATE_END) return;

    if (state == STATE_GAME_OVER) {
        ESP_LOGI("snake", "Game over, score: %d", snake_length);
        ESP_LOGI("snake", "High score: %d", high_score);

        int color = 0xFF0000;
        if (snake_length == high_score) {
            color = 0xFF0000;
        } else if (snake_length > high_score) {
            ESP_LOGI("snake", "New high score! Saving to NVS...\n");
            save_high_score(snake_length);
            color = 0xFFFF00;

            // Double blink to show new high score
            for (int i = 0; i < 2; i++) {
                for (int j = 0; j < 64; j++) {
                    draw_pixel(led_strip, j % 8, j / 8, 0x000000);
                }
                led_strip_refresh(led_strip);
                vTaskDelay(pdMS_TO_TICKS(100));
                for (int j = 0; j < 64; j++) {
                    draw_pixel(led_strip, j % 8, j / 8, 0xFFFFFF);
                }
                led_strip_refresh(led_strip);
            }
        }

        draw_number(led_strip, snake_length, color); // Show score in yellow
        led_strip_refresh(led_strip);
        state = STATE_END;

        return;
    }

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            draw_pixel(led_strip, i, j, 0xFFFFFF); // Clear pixel to prevent leaking
        }
    }

    // Draw the snake
    for (int i = 0; i < snake_length; i++) {
        draw_pixel(led_strip, snake[i].x, snake[i].y, i == 0 ? 0x00F000 : 0x00FF00); // Green for head, lighter green for body
    }

    // Draw the food
    draw_pixel(led_strip, food.x, food.y, 0xFF0000); // Red

    led_strip_refresh(led_strip);
}

// Main game loop
void snake_game_task(void *pvParameter) {
    init_snake();

    while (1) {
        // Poll buttons at twice the game speed
        update_direction();
        vTaskDelay(pdMS_TO_TICKS(GAME_SPEED_MS / 2));

        // Game update at normal speed
        update_direction();
        update_snake();
        render_game();
        vTaskDelay(pdMS_TO_TICKS(GAME_SPEED_MS / 2));
    }
}

// Function to initialize NVS
void init_nvs() {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}

// Function to save the high score to NVS
void save_high_score(uint32_t high_score) {
    nvs_handle_t nvs_handle;
    esp_err_t err;

    // Open NVS handle
    err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        return;
    }

    // Write the high score to NVS
    err = nvs_set_u32(nvs_handle, "high_score", high_score);
    if (err != ESP_OK) {
        printf("Error (%s) saving high score!\n", esp_err_to_name(err));
    }

    // Commit the changes
    err = nvs_commit(nvs_handle);
    if (err != ESP_OK) {
        printf("Error (%s) committing high score!\n", esp_err_to_name(err));
    }
    ESP_LOGI("snake", "High score saved: %d\n", high_score);

    // Close the NVS handle
    nvs_close(nvs_handle);
}

// Function to load the high score from NVS
int load_high_score() {
    nvs_handle_t nvs_handle;
    esp_err_t err;

    // Open NVS handle
    err = nvs_open("storage", NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE("snake", "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
        return high_score; // Return default high score
    }

    // Read the high score from NVS
    err = nvs_get_u32(nvs_handle, "high_score", &high_score);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGI("snake", "High score not found, using default value.\n");
    } else if (err != ESP_OK) {
        ESP_LOGE("snake", "Error (%s) reading high score!\n", esp_err_to_name(err));
    }

    // Close the NVS handle
    nvs_close(nvs_handle);

    return high_score;
}

void app_main() {
    init_nvs(); // Initialize NVS

    high_score = load_high_score();
    ESP_LOGI("snake", "Current high score: %d\n", high_score);

    init_led_matrix(&led_strip);

    init_buttons();

    xTaskCreate(snake_game_task, "snake_game_task", 4096, NULL, 5, NULL);
}
