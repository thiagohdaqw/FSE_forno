#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pid.h"
#include "state.h"

static State *global_state;

void init_global_state();
void handle_sigint(int sig);

int main() {
    init_global_state();
    signal(SIGINT, handle_sigint);

    run_user_interface(&global_state);
}

void init_global_state() {
    global_state = malloc(sizeof(State));

    memset(global_state, 0, sizeof(State));

    global_state->fan_percent = FAN_MIN_PERCENT;

    global_state->pid.kp = PID_KP_DEFAULT;
    global_state->pid.ki = PID_KI_DEFAULT;
    global_state->pid.kd = PID_KD_DEFAULT;

    global_state->reference_temperature_mode = REFERENCE_TEMPERATURE_MODE_DEFAULT;
}

void handle_sigint(int sig) {
    printf("Finalizando aplicação...\n");
    free(global_state);
    exit(1);
}
