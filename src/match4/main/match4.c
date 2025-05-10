/* Match 4 game

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "led_strip.h"
#include "sdkconfig.h"
#include "driver/gpio.h"

static const char *TAG = "match";

/* Use project configuration menu (idf.py menuconfig) to choose the GPIO to blink,
   or you can edit the following line and set a number here.
*/
#define LED_MATRIX 5 //CONFIG_LED_MATRIX

#define ROWS 8
#define COLS 8

#define P1_COLOUR         0xffff00 // 0xffff00
#define P1_SELECT_COLOUR  0xecec53

#define P2_COLOUR         0x00b7ef
#define P2_SELECT_COLOUR  0x99d9ea

#define WHITE_COLOR       0xffffffff
#define BLANK             0


#define GPIO_DPAD_UP        CONFIG_GPIO_DPAD_UP
#define GPIO_DPAD_DOWN      CONFIG_GPIO_DPAD_DOWN
#define GPIO_DPAD_LEFT      CONFIG_GPIO_DPAD_LEFT
#define GPIO_DPAD_RIGHT     CONFIG_GPIO_DPAD_RIGHT
#define GPIO_DPAD_BUTTON    CONFIG_GPIO_DPAD_BUTTON

#define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_DPAD_UP) | (1ULL<<GPIO_DPAD_DOWN) |    \
                             (1ULL<<GPIO_DPAD_LEFT) | (1ULL<<GPIO_DPAD_RIGHT) | \
                             (1ULL<<GPIO_DPAD_BUTTON))

uint32_t gpio_pins[5] = {
    GPIO_DPAD_UP,
    GPIO_DPAD_DOWN,
    GPIO_DPAD_LEFT,
    GPIO_DPAD_RIGHT,
    GPIO_DPAD_BUTTON
};

uint32_t players[2] = {
  P1_COLOUR,
  P2_COLOUR
};

uint32_t players_select[2] = {
  P1_SELECT_COLOUR,
  P2_SELECT_COLOUR
};

uint32_t board[ROWS][COLS] = {
  0
};

uint32_t one_player_scr[ROWS][COLS] = {
  { 0xff00b7ef, 0xffffffff, 0xffffffff, 0xffffffff, 0xff00b7ef, 0xffffffff, 0xffffffff, 0xffffffff },
  { 0xffffffff, 0xff00b7ef, 0xffffffff, 0xff00b7ef, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff },
  { 0xffffffff, 0xffffffff, 0xff00b7ef, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff },
  { 0xffffffff, 0xffffffff, 0xffed1c24, 0xffffffff, 0xffed1c24, 0xffed1c24, 0xffed1c24, 0xffffffff },
  { 0xffffffff, 0xffed1c24, 0xffed1c24, 0xffffffff, 0xffffffff, 0xffffffff, 0xffed1c24, 0xffffffff },
  { 0xffffffff, 0xffffffff, 0xffed1c24, 0xffffffff, 0xffed1c24, 0xffed1c24, 0xffed1c24, 0xffffffff },
  { 0xffffffff, 0xffffffff, 0xffed1c24, 0xffffffff, 0xffed1c24, 0xffffffff, 0xffffffff, 0xffffffff },
  { 0xffffffff, 0xffffffff, 0xffed1c24, 0xffffffff, 0xffed1c24, 0xffed1c24, 0xffed1c24, 0xffffffff }
};

uint32_t two_player_scr[ROWS][COLS] = {
  { 0xffffffff, 0xffffffff, 0xffffffff, 0xff00b7ef, 0xffffffff, 0xffffffff, 0xffffffff, 0xff00b7ef },
  { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xff00b7ef, 0xffffffff, 0xff00b7ef, 0xffffffff },
  { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xff00b7ef, 0xffffffff, 0xffffffff },
  { 0xffffffff, 0xffffffff, 0xffed1c24, 0xffffffff, 0xffed1c24, 0xffed1c24, 0xffed1c24, 0xffffffff },
  { 0xffffffff, 0xffed1c24, 0xffed1c24, 0xffffffff, 0xffffffff, 0xffffffff, 0xffed1c24, 0xffffffff },
  { 0xffffffff, 0xffffffff, 0xffed1c24, 0xffffffff, 0xffed1c24, 0xffed1c24, 0xffed1c24, 0xffffffff },
  { 0xffffffff, 0xffffffff, 0xffed1c24, 0xffffffff, 0xffed1c24, 0xffffffff, 0xffffffff, 0xffffffff },
  { 0xffffffff, 0xffffffff, 0xffed1c24, 0xffffffff, 0xffed1c24, 0xffed1c24, 0xffed1c24, 0xffffffff }
};

static led_strip_handle_t led_strip;

static void draw_led_matrix(uint32_t board[COLS][ROWS]);

// Initialize the board with empty spaces
void init_board() {
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLS; j++)
            board[i][j] = 0;
}

// Print the game board
void print_board(uint32_t board[COLS][ROWS])
{
    char symbols[] = {' ', 'X', 'O'};
    printf("\n");
    for (int i = 0; i < COLS; i++) {
        printf(" %d", i);
    }
    printf("\n");

    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
          if(board[i][j] == BLANK)
            printf("|%c", symbols[0]);
          else if(board[i][j] == players[0])
            printf("|%c", symbols[1]) ;
          else
            printf("|%c", symbols[2]);
        }
        printf("|\n");
    }
    for (int i = 0; i < COLS * 2 + 1; i++) printf("-");
    printf("\n");
    draw_led_matrix(board);
}

// Drop a piece in the selected column
int put_mark(int col, uint32_t piece)
{
    if (col < 0 || col >= COLS) return 0;

    for (int i = ROWS - 1; i >= 0; i--) {
        if (board[i][col] == BLANK) {
            board[i][col] = piece;
            return 0;
        }
    }
    return 1;
}

// Check for 4 in a row in any direction
int check_winner(uint32_t piece)
{
    int delay_ms = 300;
    // Horizontal
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j <= COLS - 4; j++) {
            if (board[i][j] == piece &&
                board[i][j+1] == piece &&
                board[i][j+2] == piece &&
                board[i][j+3] == piece) {

                    for (int k = 0; k < 3; k++) {

                        board[i][j] = WHITE_COLOR;
                        board[i][j+1] = WHITE_COLOR;
                        board[i][j+2] = WHITE_COLOR;
                        board[i][j+3] = WHITE_COLOR;
                        draw_led_matrix(board);
                        vTaskDelay(pdMS_TO_TICKS(delay_ms));

                        board[i][j] = piece;
                        board[i][j+1] = piece;
                        board[i][j+2] = piece;
                        board[i][j+3] = piece;
                        draw_led_matrix(board);
                        vTaskDelay(pdMS_TO_TICKS(delay_ms));

                    }
                    return 1;
                }
        }
    }

    // Vertical
    for (int i = 0; i <= ROWS - 4; i++) {
        for (int j = 0; j < COLS; j++) {
            if (board[i][j] == piece &&
                board[i+1][j] == piece &&
                board[i+2][j] == piece &&
                board[i+3][j] == piece) {

                for (int k = 0; k < 3; k++) {
                    board[i][j] = WHITE_COLOR;
                    board[i+1][j] = WHITE_COLOR;
                    board[i+2][j] = WHITE_COLOR;
                    board[i+3][j] = WHITE_COLOR;
                    draw_led_matrix(board);
                    vTaskDelay(pdMS_TO_TICKS(delay_ms));

                    board[i][j] = piece;
                    board[i+1][j] = piece;
                    board[i+2][j] = piece;
                    board[i+3][j] = piece;
                    draw_led_matrix(board);
                    vTaskDelay(pdMS_TO_TICKS(delay_ms));
                }

                return 1;
            }
        }
    }

    // Diagonal down-right
    for (int i = 0; i <= ROWS - 4; i++) {
        for (int j = 0; j <= COLS - 4; j++) {
            if (board[i][j] == piece &&
                board[i+1][j+1] == piece &&
                board[i+2][j+2] == piece &&
                board[i+3][j+3] == piece) {

                for (int k = 0; k < 3; k++) {
                    board[i][j] = WHITE_COLOR;
                    board[i+1][j+1] = WHITE_COLOR;
                    board[i+2][j+2] = WHITE_COLOR;
                    board[i+3][j+3] = WHITE_COLOR;
                    draw_led_matrix(board);
                    vTaskDelay(pdMS_TO_TICKS(delay_ms));

                    board[i][j] = piece;
                    board[i+1][j+1] = piece;
                    board[i+2][j+2] = piece;
                    board[i+3][j+3] = piece;
                    draw_led_matrix(board);
                    vTaskDelay(pdMS_TO_TICKS(delay_ms));
                }

                return 1;
            }
        }
    }

    // Diagonal up-right
    for (int i = 3; i < ROWS; i++) {
        for (int j = 0; j <= COLS - 4; j++) {
            if (board[i][j] == piece &&
                board[i-1][j+1] == piece &&
                board[i-2][j+2] == piece &&
                board[i-3][j+3] == piece) {

                for (int k = 0; k < 3; k++) {
                    board[i][j] = WHITE_COLOR;
                    board[i-1][j+1] = WHITE_COLOR;
                    board[i-2][j+2] = WHITE_COLOR;
                    board[i-3][j+3] = WHITE_COLOR;
                    draw_led_matrix(board);
                    vTaskDelay(pdMS_TO_TICKS(delay_ms));

                    board[i][j] = piece;
                    board[i-1][j+1] = piece;
                    board[i-2][j+2] = piece;
                    board[i-3][j+3] = piece;
                    draw_led_matrix(board);
                    vTaskDelay(pdMS_TO_TICKS(delay_ms));
                }

                return 1;
            }
        }
    }

    return 0;
}

// Check if the board is full (draw)
static int is_board_full()
{
    for (int j = 0; j < COLS; j++)
      {
        if (board[0][j] == BLANK)
          {
            return 0;
          }
      }
    return 1;
}


void get_rgb(uint32_t color, uint8_t *r, uint8_t *g, uint8_t *b) {
    *r = (color >> 16) & 0xFF;  // Extract red component
    *g = (color >> 8) & 0xFF;   // Extract green component
    *b = color & 0xFF;          // Extract blue component
}

static void draw_led_matrix(uint32_t board[COLS][ROWS])
{
    uint8_t r, g, b;
    int index = 0;
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            get_rgb(board[i][j], &r, &g, &b);
            led_strip_set_pixel(led_strip, index, r, g, b);
            index ++;
        }
    }
    led_strip_refresh(led_strip);
}


int ai_move(uint32_t piece, uint32_t opponent_piece) {

    // Try to win
    for (int col = 0; col < COLS; col++) {
        for (int row = ROWS - 1; row >= 0; row--) {
            if (board[row][col] == BLANK) {
                board[row][col] = piece;
                if (check_winner(piece)) {
                    return col;
                }
                board[row][col] = 0; // undo move
                break;
            }
        }
    }

    // Try to block opponent
    for (int col = 0; col < COLS; col++) {
        for (int row = ROWS - 1; row >= 0; row--) {
            if (board[row][col] == BLANK) {
                board[row][col] = opponent_piece;
                if (check_winner(opponent_piece)) {
                    board[row][col] = 0;
                    return col;
                }
                board[row][col] = 0;
                break;
            }
        }
    }

    // Choose random column
    int valid_cols[COLS], count = 0;
    for (int col = 0; col < COLS; col++) {
        if (board[0][col] == BLANK) {
            valid_cols[count++] = col;
        }
    }

    if (count > 0) {
        return valid_cols[rand() % count];
    }

    return -1; // No valid moves
}

int find_empty_col(int col, int dir)
{
    int  i = 0;
    while (i < COLS) {
        if ((col == 7) && (dir == 1)) {
            col = 0;
        } else if ((col == BLANK) && (dir == -1)) {
            col = 7;
        } else {
            col = col + dir;
        }
        if (board[0][col] == BLANK) {
            break;
        }
        i++;
    }
    return col;
}

static int take_input(int turn, int *col)
{
    uint8_t column = find_empty_col(-1, 1);
    bool refresh_board = true;
    printf("Player %d, choose column (0-%d): ", turn + 1, COLS - 1);

    while (!gpio_get_level(GPIO_DPAD_BUTTON))
    {
        if (gpio_get_level(GPIO_DPAD_RIGHT)) {
            column = find_empty_col(column, 1);
            refresh_board = true;
            printf("right button pressed: %d\n", column);
            vTaskDelay(pdMS_TO_TICKS(70));
        }
        if (gpio_get_level(GPIO_DPAD_LEFT)) {
            column = find_empty_col(column, -1);
            refresh_board = true;
            printf("left button pressed: %d\n", column);
            vTaskDelay(pdMS_TO_TICKS(70));
        }

        if (refresh_board) {
            refresh_board = false;
            put_mark(column, players_select[turn]);
            print_board(board);
            int j = 0;
            while (board[j][column] != players_select[turn]) {
                j++;
            }
            board[j][column] = 0;
        }
        vTaskDelay(pdMS_TO_TICKS(70));

    }
    //while(gpio_get_level(GPIO_DPAD_BUTTON));
    vTaskDelay(pdMS_TO_TICKS(200));

    printf("go button pressed\n");
    *col = column;
    return 0;
}

static bool game_enterance(void)
{
    bool game_mode_one = true;

    draw_led_matrix(one_player_scr);
    while (!gpio_get_level(GPIO_DPAD_BUTTON)) {
        if (gpio_get_level(GPIO_DPAD_LEFT) || gpio_get_level(GPIO_DPAD_RIGHT)) {
            game_mode_one = !game_mode_one;
            draw_led_matrix(game_mode_one == true ? one_player_scr : two_player_scr);
        }
        vTaskDelay(pdMS_TO_TICKS(170));
    }
    vTaskDelay(pdMS_TO_TICKS(200));
    return game_mode_one;
}

static void configure_gpio(void)
{
    //zero-initialize the config structure.
    gpio_config_t io_conf = {};
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_INPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
}

static void configure_led(void)
{
    ESP_LOGI(TAG, "Example configured to blink addressable LED!");
    /* LED strip initialization with the GPIO and pixels number*/
    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_MATRIX,
        .max_leds = ROWS * COLS, // at least one LED on board
    };

    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));

    /* Set all LED off to clear all pixels */
    led_strip_clear(led_strip);
}

void print_winner(uint8_t turn) {
    for (int i = 0; i < COLS; i++) {
        for (int j = 0; j < ROWS; j++) {
            board[i][j] = players[turn];
        }
    }
    vTaskDelay(pdMS_TO_TICKS(20));
    print_board(board);
}

void app_main(void)
{
    int col = 0;
    int turn = 0;
    bool one_player_mode = true;
    char playerSymbol[2] = {'X', 'O'};
    int ret;

    /* Configure the peripheral according to the LED type */
    configure_led();
    configure_gpio();

restart:
    srand(time(NULL));

    ret = 0;
    turn = 0;
    col = 0;
    one_player_mode = true;
    init_board();

    one_player_mode = game_enterance();
    while (1) {
        print_board(board);


        if ((one_player_mode && turn == BLANK) || !one_player_mode) {
            ret = take_input(turn, &col);
            if (ret != 0)
            {
                break;
            }
        } else {
          col = ai_move(players[turn], players[(turn + 1) % 2]);
          printf("AI chooses column %d\n", col);
        }

        ret = put_mark(col, players[turn]);
        if (ret != 0) {
            printf("Column full or invalid. Try again.\n");
            continue;
        }

        if (check_winner(players[turn])) {
            if (one_player_mode && turn == 1) {
                printf("Player %d (%c) wins!\n", turn + 1, playerSymbol[turn]);
            } else {
                printf("AI (%c) wins!\n", playerSymbol[turn]);
            }

            print_board(board);

            print_winner(turn);
            vTaskDelay(pdMS_TO_TICKS(3000));
            break;
        }

        if (is_board_full()) {
            print_board(board);
            printf("It's a draw!\n");
            break;
        }

        turn = (turn + 1) % 2; // switch turns

    }

    goto restart;
}
