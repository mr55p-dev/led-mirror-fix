/**
 * @file
 * @brief controller logic for setting the different states of a two-light LED
 * strip. The strip should be wired such that there is a single 3.3v VIN line,
 * and two grounded VOUT lines, one for each of the colour shades. A button can
 * be wired to pin 27 to toggle between these states.
 */

#include <hardware/gpio.h>
#include <pico/time.h>
#include <stdint.h>

// set ground pin high for off or low for on
#define OFF true
#define ON false

// Setup the pins
#define GPIO_OUT_WARM 19  // warm light
#define GPIO_OUT_COOL 18  // cool light
#define GPIO_IN_BUTTON 27 // button in

int lamp_state_active_idx = 0;
bool lamp_states[4][2] = {
    {OFF, OFF}, // off
    {ON, ON},   // on
    {OFF, ON},  // cool
    {ON, OFF},  // warm
};

// Perform initialisation
int output_init(int pin) {
  gpio_init(pin);
  gpio_set_dir(pin, GPIO_OUT);
  gpio_set_pulls(pin, false, true); // pull-down
  return PICO_OK;
}

int input_init(int pin) {
  gpio_init(pin);
  gpio_set_dir(GPIO_IN_BUTTON, GPIO_IN);
  gpio_set_pulls(pin, false, true); // pull-down
  return PICO_OK;
}

void set_state(bool state[2]) {
  gpio_put(GPIO_OUT_WARM, state[0]);
  gpio_put(GPIO_OUT_COOL, state[1]);
}

void irq_handle_state_shift(uint gpio, uint32_t event_mask) {
  if (gpio == GPIO_IN_BUTTON) {
    lamp_state_active_idx++;
    lamp_state_active_idx = lamp_state_active_idx % 4;
    set_state(lamp_states[lamp_state_active_idx]);
  }
}

int main() {
  output_init(GPIO_OUT_WARM);
  output_init(GPIO_OUT_COOL);
  input_init(GPIO_IN_BUTTON);

  set_state(lamp_states[lamp_state_active_idx]);

  gpio_set_irq_enabled_with_callback(GPIO_IN_BUTTON,
                                     GPIO_IRQ_EDGE_FALL & GPIO_IRQ_EDGE_RISE,
                                     true, &irq_handle_state_shift);

  while (true) {
    sleep_ms(UINT32_MAX);
  }
}
