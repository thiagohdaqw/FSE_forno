#ifndef __STATE_H__
#define __STATE_H__

#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>

#include <bme280.h>

#define ESP32_ADDRESS 0x01
#define APP_ADDRESS 0x00
#define UART_DEVICE "/dev/serial0"

#define REFERENCE_TEMPERATURE_MODE_UART 0
#define REFERENCE_TEMPERATURE_MODE_FILE 1
#define REFERENCE_TEMPERATURE_MODE_DEBUG 2
#define REFERENCE_TEMPERATURE_MODE_DEFAULT REFERENCE_TEMPERATURE_MODE_UART

#define WORKING_EVENT_SIZE 1

typedef struct {
    float kp;
    float ki;
    float kd;
    int value;
} PID;

typedef struct {
    float value;
    char mode;
    char is_debug;
    pthread_t tid;
    FILE *fd;
} ReferenceTemperature;

typedef struct {
    float resistor_percent;
    float fan_percent;

    float intern_temperature;
    float extern_temperature;

    ReferenceTemperature reference_temperature;

    char is_working;
    char is_heating;

    sem_t working_event;
    sem_t heating_event;
    PID pid;
    struct bme280_dev *extern_temperature_device;
} State;

void set_default_state(State *state);

#endif