#ifndef PLOTTER_DRV_ENCODER_H
#define PLOTTER_DRV_ENCODER_H

#include <stdint.h>

struct EncoderState {
    int32_t position;
    bool button_pressed;
};

void drv_encoder_init();
void drv_encoder_update();
EncoderState drv_encoder_get_state();
int32_t drv_encoder_read_delta();

#endif
