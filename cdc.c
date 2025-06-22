#include <pico/stdlib.h>
#include <pico/time.h>
#include <hardware/gpio.h>
#include <stdlib.h>
#include <bsp/board_api.h>
#include <tusb.h>

#include <pico/stdio.h>

// Define relay pin pairs (A, B)
const uint8_t relay_pins[5][2] = {{5, 4}, {1, 0}, {3, 2}, {9, 8}, {7, 6}};
uint8_t current_route = 0;
// Define four routes: GPIOs to pulse for each route
const uint8_t routes[4][4] = {
    {5, 3, 9, 7},  // Route 1: {A, X, A, A, A}
    {4, 2, 9, 7},  // Route 2: {B, X, B, A, A}
    {1, 3, 8, 6},  // Route 3: {X, A, A, B, B}
    {0, 2, 8, 6}   // Route 4: {X, B, B, B, B}
};

const uint8_t led_pins[5] = {10, 11, 12, 13, 14};
const uint8_t button_pin = 15;
const char charbuf[4] = {'1', '2', '3', '4'};

// Set specified route
void set_route(uint8_t route_index) {
    if (route_index < 4) {
        for (uint8_t i = 0; i < 4; i++) {
            gpio_put(routes[route_index][i],1);
        }
        sleep_ms(10);
        for (uint8_t i = 0; i < 4; i++) {
            gpio_put(routes[route_index][i],0);
        }
        // Update LEDs (open-drain: 0 = on, 1 = off)
        for (uint8_t i = 0; i < 4; i++) {
            gpio_put(led_pins[i], i == route_index ? 0 : 1);
        }
    }
}

// Initialize hardware
void init_hardware() {
    // Initialize relay pins
    for (uint8_t i = 0; i < 5; i++) {
        gpio_init(relay_pins[i][0]);
        gpio_init(relay_pins[i][1]);
        gpio_set_dir(relay_pins[i][0], GPIO_OUT);
        gpio_set_dir(relay_pins[i][1], GPIO_OUT);
        gpio_put(relay_pins[i][0], 0);
        gpio_put(relay_pins[i][1], 0);
    }
    // Initialize LED pins (open-drain)
    for (uint8_t i = 0; i < 5; i++) {
        gpio_init(led_pins[i]);
        gpio_set_dir(led_pins[i], GPIO_OUT);
        gpio_set_function(led_pins[i], GPIO_FUNC_SIO);
        gpio_set_pulls(led_pins[i], false, false); // Disable pull-up/down
        gpio_put(led_pins[i], 1); // Initial off
    }
    // Initialize button pin (pull-up)
    gpio_init(button_pin);
    gpio_set_dir(button_pin, GPIO_IN);
    gpio_pull_up(button_pin);
}

int main(void)
{
    // Initialize TinyUSB stack
    board_init();
    tusb_init();

    // TinyUSB board init callback after init
    if (board_init_after_tusb) {
        board_init_after_tusb();
    }
    init_hardware();
    set_route(current_route);

    // Button state
    bool prev_button = true;
    char buffer[8] = {0};
    uint8_t buf_idx = 0;


    // main run loop
    while (1) {
        // TinyUSB device task | must be called regurlarly
        tud_task();
        // Button press detection (debouncing)
        bool current_button = gpio_get(button_pin);
        if (!current_button && prev_button) {
            sleep_ms(20); // Debounce
            if (!gpio_get(button_pin)) {
                current_route = (current_route + 1) % 4;
                set_route(current_route);
                while (!gpio_get(button_pin)) { // Wait for release
                    sleep_ms(10);
                }
            }
        }
        prev_button = current_button;
    }

    // indicate no error
    return 0;
}

// callback when data is received on a CDC interface
void tud_cdc_rx_cb(uint8_t itf)
{
    // allocate buffer for the data in the stack
    gpio_put(led_pins[4], 0);
    uint8_t buf[CFG_TUD_CDC_RX_BUFSIZE];
    uint32_t count = tud_cdc_n_read(itf, buf, sizeof(buf));

    // check if the data was received on the second cdc interface
    if (itf == 0) {
        // process the received data
        buf[count] = 0; // null-terminate the string

        if(buf[0] == '0')
        {
            tud_cdc_n_write(itf, (uint8_t const *)(charbuf + current_route), 1);
        }else if(buf[0] >= '1' && buf[0] <= '4') {
            current_route = buf[0] - '1';
            set_route(buf[0] - '1'); // convert char to int (1-4)
            tud_cdc_n_write(itf, (uint8_t const *) "OK\r\n", 4);
        }else {
            // if the command is invalid, send an error message
            tud_cdc_n_write(itf, (uint8_t const *) "Invalid Command\r\n", 18);
        }
        tud_cdc_n_write_flush(itf);
    }
    gpio_put(led_pins[4], 1);
}
