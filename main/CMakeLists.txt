idf_component_register(SRCS "snake.c" "numbers.c" "led.c"
                       PRIV_REQUIRES driver esp_driver_gpio esp_timer esp_driver_rmt nvs_flash
                       INCLUDE_DIRS ".")
