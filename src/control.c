#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "commands.h"
#include "state.h"
#include "control.h"

typedef struct {
    pthread_t tid;
    State *state;
    CommandArgs *commands;
} ControlArgs;

void *control_worker(void *args);

void init_control(State *state, CommandArgs *commands) {
    ControlArgs *args = malloc(sizeof(ControlArgs));
    args->commands = commands;
    args->state = state;
    pthread_create(&args->tid, NULL, control_worker, args);
}

void *control_worker(void *args) {
    ControlArgs *cargs = (ControlArgs *)args;
    State *state = cargs->state;
    CommandArgs *commands = cargs->commands;

    while (1) {
        sem_wait(&state->working_event);

        if (!state->is_working) {
            continue;
        }

        printf("Calculando PID\n");

        if (state->is_working) {
            sem_post(&state->working_event);
        }

        sleep(CONTROL_POLLING_S);
    }
    printf("FIM\n");
}
