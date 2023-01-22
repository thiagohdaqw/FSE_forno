#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "extern_temperature.h"
#include "bme.h"

typedef struct {
    pthread_t tid;
    State *state;
} ExternTemperatureArgs;

void *run_extern_temperature_worker(void *args);

void init_extern_temperature(State *state) {
    ExternTemperatureArgs *args = malloc(sizeof(ExternTemperatureArgs));
    
    args->state = state;

    state->extern_temperature_device = init_bme280(EXTERN_TEMPERATURE_ADDRESS);

    pthread_create(&args->tid, NULL, run_extern_temperature_worker, args);
}

void *run_extern_temperature_worker(void *args) {
    ExternTemperatureArgs *targs = (ExternTemperatureArgs *)args;
    State *state = targs->state;
    int8_t result;
    float temperature;
    while (1) {
        sem_wait(&state->working_event);

        if (!state->is_working) {
            continue;
        }

        result = get_sensor_data_forced_mode(state->extern_temperature_device, &temperature);
        if (result == BME280_OK) {
            state->extern_temperature = temperature;
            send_command(COMMAND_SEND_EXTERN_TEMPERATURE, state);
        }

        if (state->is_working) {
            sem_post(&state->working_event);
        }

        sleep(EXTERN_TEMPERATURE_POLLING_SECONDS);
    }
}