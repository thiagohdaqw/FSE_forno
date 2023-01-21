#ifndef __STATE_H__
#define __STATE_H__

#include <string.h>

#define ESP32_ADDRESS 0x01
#define APP_ADDRESS 0x00
#define UART_DEVICE "/dev/serial0"

#define FAN_MIN_PERCENT 40.0

#define REFERENCE_TEMPERATURE_MODE_UART 0
#define REFERENCE_TEMPERATURE_MODE_FILE 1
#define REFERENCE_TEMPERATURE_MODE_DEBUG 2
#define REFERENCE_TEMPERATURE_MODE_DEFAULT REFERENCE_TEMPERATURE_MODE_UART

#define PID_KP_DEFAULT 30.0
#define PID_KI_DEFAULT 0.2
#define PID_KD_DEFAULT 400.0

typedef struct {
    float kp;
    float ki;
    float kd;
    float value;
} PID;

typedef struct {
    float resistor_percent;
    float fan_percent;

    float intern_temperature;
    float extern_temperature;

    float reference_temperature;
    char reference_temperature_mode;
    char reference_temperature_debug_mode;

    char is_working;
    char is_heating;
    PID pid;
} State;

void set_default_state(State *state);

#endif