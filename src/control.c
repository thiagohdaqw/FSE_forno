#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "pid.h"
#include "state.h"
#include "control.h"
#include "commands.h"

typedef struct {
    pthread_t tid;
    State *state;
    CommandArgs *commands;
} ControlArgs;

void *run_control_worker(void *args);

void init_control(State *state, CommandArgs *commands) {
    ControlArgs *args = malloc(sizeof(ControlArgs));
    args->commands = commands;
    args->state = state;
    pthread_create(&args->tid, NULL, run_control_worker, args);
}

void *run_control_worker(void *args) {
    ControlArgs *cargs = (ControlArgs *)args;
    State *state = cargs->state;
    CommandArgs *commands = cargs->commands;
    double pid;

    while (1) {
        sem_wait(&state->working_event);

        if (!state->is_working) {
            continue;
        }

        pid_configura_constantes(state->pid.kp, state->pid.ki, state->pid.kd);
        pid_atualiza_referencia(state->reference_temperature);
        pid = pid_controle(state->intern_temperature);

        printf("REF=%f IN=%f PID=%f\n", state->reference_temperature, state->intern_temperature, pid);

        if (state->is_working) {
            sem_post(&state->working_event);
        }

        sleep(CONTROL_POLLING_SECONDS);
    }
}
