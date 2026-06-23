#ifndef PLOTTER_HAL_ESP32_H
#define PLOTTER_HAL_ESP32_H

// ESP32-S2 board-specific definitions. Change these for your board only.

#define X_IN1_PIN 2
#define X_IN2_PIN 3
#define X_IN3_PIN 4
#define X_IN4_PIN 5

#define Y_IN1_PIN 6
#define Y_IN2_PIN 7
#define Y_IN3_PIN 8
#define Y_IN4_PIN 9

#define Z_IN1_PIN 10
#define Z_IN2_PIN 11
#define Z_IN3_PIN 12
#define Z_IN4_PIN 13

#define X_LIMIT_PIN 34
#define Y_LIMIT_PIN 35
#define Z_LIMIT_PIN 36

// Optional second end buttons for printer-style automatic travel calibration.
// Leave as 255 when not wired.
#define X_MAX_LIMIT_PIN 255
#define Y_MAX_LIMIT_PIN 255
#define Z_MAX_LIMIT_PIN 255

#define ENCODER_A_PIN 14
#define ENCODER_B_PIN 15
#define ENCODER_BUTTON_PIN 16

#define OLED_SDA 17
#define OLED_SCL 18
#define OLED_I2C_ADDRESS 0x3C

#define WIFI_STATUS_LED_PIN 21

#define PEN_SERVO_PIN 33

#endif
