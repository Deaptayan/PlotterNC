#include "drv_encoder.h"

#include "../hal/hal.h"
#include "../hal/hal_platform.h"

static EncoderState state = {0, false};
static HalPinLevel last_a = HAL_LOW;
static int32_t last_reported_position = 0;

void drv_encoder_init() {
    hal_gpio_mode(ENCODER_A_PIN, HAL_PIN_INPUT_PULLUP);
    hal_gpio_mode(ENCODER_B_PIN, HAL_PIN_INPUT_PULLUP);
    hal_gpio_mode(ENCODER_BUTTON_PIN, HAL_PIN_INPUT_PULLUP);
    last_a = hal_gpio_read(ENCODER_A_PIN);
}

void drv_encoder_update() {
    const HalPinLevel a = hal_gpio_read(ENCODER_A_PIN);
    const HalPinLevel b = hal_gpio_read(ENCODER_B_PIN);

    if (a != last_a && a == HAL_HIGH) {
        state.position += (b == HAL_LOW) ? 1 : -1;
    }

    last_a = a;
    state.button_pressed = hal_gpio_read(ENCODER_BUTTON_PIN) == HAL_LOW;
}

EncoderState drv_encoder_get_state() {
    return state;
}

int32_t drv_encoder_read_delta() {
    const int32_t delta = state.position - last_reported_position;
    last_reported_position = state.position;
    return delta;
}
