#ifndef __DEFS_H__
#define __DEFS_H__

#define FAN_MIN_PERCENT 40.0

#define REFERENCE_TEMPERATURE_MODE_DEBUG 0
#define REFERENCE_TEMPERATURE_MODE_UART 1
#define REFERENCE_TEMPERATURE_MODE_FILE 2
#define REFERENCE_TEMPERATURE_MODE_DEFAULT REFERENCE_TEMPERATURE_MODE_UART

typedef struct
{
    float kp;
    float ki;
    float kd;
} PID;

typedef struct
{
    float intern_temperature;
    float extern_temperature;
    float user_temperature;
    float resistor_percent;
    float fan_percent;
    float reference_temperature;
    char  reference_temperature_mode;
    PID pid;
} State;

#endif