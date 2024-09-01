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
#define GPIO_IN_BUTTON 28 // button in

#define DEBOUNCE_INTERVAL_US 100000 // 100ms
absolute_time_t debounce_last_hit = 0;

int lamp_state_active_idx = 2;
bool lamp_states[4][2] = {
    {OFF, OFF}, // off
    {ON, ON},   // on
    {OFF, ON},  // cool
    {ON, OFF},  // warm
};
void lamp_state_next_idx() {
  lamp_state_active_idx++;
  lamp_state_active_idx = lamp_state_active_idx % 4;
}

// Perform initialisation
int gpio_init_output(int pin) {
  gpio_init(pin);
  gpio_set_dir(pin, GPIO_OUT);
  gpio_set_pulls(pin, false, true); // pull-down
  return PICO_OK;
}

int gpio_init_input(int pin) {
  gpio_init(pin);
  gpio_set_dir(pin, GPIO_IN);
  gpio_set_pulls(pin, false, true); // pull-down
  return PICO_OK;
}

void set_state(bool state[2]) {
  gpio_put(GPIO_OUT_WARM, state[0]);
  gpio_put(GPIO_OUT_COOL, state[1]);
}

void irq_handle_state_shift(uint gpio, uint32_t event_mask) {
  absolute_time_t now = get_absolute_time();
  absolute_time_t debounce_window = debounce_last_hit + DEBOUNCE_INTERVAL_US;
  if (debounce_window > now) {
    return;
  }

  if (gpio != GPIO_IN_BUTTON) {
    return;
  }
  debounce_last_hit = now;
  lamp_state_next_idx();
  set_state(lamp_states[lamp_state_active_idx]);
}

int main() {
  gpio_init_output(GPIO_OUT_WARM);
  gpio_init_output(GPIO_OUT_COOL);
  gpio_init_input(GPIO_IN_BUTTON);

  set_state(lamp_states[lamp_state_active_idx]);

  gpio_set_irq_enabled_with_callback(GPIO_IN_BUTTON, GPIO_IRQ_EDGE_RISE, true,
                                     &irq_handle_state_shift);

  while (true) {
    sleep_ms(UINT32_MAX);
  }
}
