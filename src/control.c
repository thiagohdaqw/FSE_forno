#include <pthread.h>
#include <semaphore.h>
#include <softPwm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wiringPi.h>

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
        pid_atualiza_referencia(state->reference_temperature);
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

        printf("REF=%f IN=%f PID=%d RP=%d FP=%d\n", state->reference_temperature, state->intern_temperature, state->pid.value,
               MAX(0, state->pid.value), pid);
        // printf("KP=%f KI=%f KD=%f\n", state->pid.kp, state->pid.ki, state->pid.kd);

        if (state->is_working && state->is_heating) {
            sem_post(&state->heating_event);
        }

        sleep(CONTROL_POLLING_SECONDS);
    }
}
