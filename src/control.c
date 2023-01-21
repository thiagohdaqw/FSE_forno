#include <wiringPi.h>
#include <softPwm.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "commands.h"
#include "control.h"
#include "pid.h"
#include "state.h"

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

typedef struct {
    pthread_t tid;
    State *state;
} ControlArgs;

void *run_control_worker(void *args);

void stop_control() {
    softPwmWrite(RESISTOR_PIN, 0);
    softPwmWrite(FAN_PIN, 0);
}

void init_control(State *state) {
    ControlArgs *args = malloc(sizeof(ControlArgs));

    args->state = state;

    wiringPiSetup();
    pinMode(RESISTOR_PIN, OUTPUT);
    pinMode(FAN_PIN, OUTPUT);
    softPwmCreate(RESISTOR_PIN, 0, 100);
    softPwmCreate(FAN_PIN, 0, 100);

    pthread_create(&args->tid, NULL, run_control_worker, args);
}

void *run_control_worker(void *args) {
    ControlArgs *cargs = (ControlArgs *)args;
    State *state = cargs->state;
    int pid;

    while (1) {
        if (!state->is_working || !state->is_heating) {
            stop_control();
        }

        sem_wait(&state->heating_event);

        if (!state->is_working || !state->is_heating) {
            continue;
        }

        pid_configura_constantes(state->pid.kp, state->pid.ki, state->pid.kd);
        pid_atualiza_referencia(state->reference_temperature.value);
        state->pid.value = (int)pid_controle(state->intern_temperature);
        pid = state->pid.value;

        softPwmWrite(RESISTOR_PIN, MAX(0, pid));
        if (pid < 0) {
            pid *= -1;
            pid = pid < FAN_MIN_PERCENT ? FAN_MIN_PERCENT : pid;
        } else {
            pid = FAN_MIN_PERCENT;
        }

        softPwmWrite(FAN_PIN, pid);
        send_command(COMMAND_SEND_CONTROL, state);

        printf("REF=%f IN=%f PID=%d RP=%d FP=%d\n", state->reference_temperature.value, state->intern_temperature, state->pid.value,
               MAX(0, state->pid.value), pid);

        if (state->is_working && state->is_heating) {
            sem_post(&state->heating_event);
        }

        usleep(CONTROL_POLLING_US);
    }
}

void *run_file_mode(void *args) {
    State *state = (State *)args;
    ReferenceTemperature *rf = &state->reference_temperature;
    char header_readed = 0;
    pthread_t tid = rf->tid;
    int sleep_seconds;
    char time[15], temperature[15];
    
    rf->fd = fopen(REFERENCE_TEMPERATURE_FILE_PATH, "r");
    if (rf->fd == NULL) {
        fprintf(stderr, "Failed to open reference temperature file\n");
        return NULL;
    }

    while (fscanf(rf->fd, "%[^,],%s ", time, temperature) == 2) {
        if (!header_readed) {
            header_readed = 1;
            continue;
        }
        if (rf->mode != REFERENCE_TEMPERATURE_MODE_FILE || rf->tid != tid || !state->is_heating || !state->is_working) {
            break;
        }
        rf->value = (float)atof(temperature);
        sleep_seconds = atoi(time);
        printf("%f %d\n", rf->value, sleep_seconds);
        send_command(COMMAND_SEND_REFERENCE_TEMPERATURE, state);
        sleep(sleep_seconds);
    }
    if (rf->fd) {
        fclose(rf->fd);
        rf->fd = NULL;
    }
    return NULL;
}

void stop_file_mode(State *state) {
    ReferenceTemperature *rf = &state->reference_temperature;
    if (rf->tid) {
        pthread_cancel(rf->tid);
        rf->tid = 0;
    }
    if (rf->fd) {
        fclose(rf->fd);
        rf->fd = NULL;
    }
}

void start_file_mode(State *state) {
    stop_file_mode(state);
    pthread_create(&state->reference_temperature.tid, NULL, run_file_mode, state);
}
