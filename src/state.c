#include "state.h"

void set_default_state(State *state) {
    memset(state, 0, sizeof(State));

    state->pid.kp = PID_KP_DEFAULT;
    state->pid.ki = PID_KI_DEFAULT;
    state->pid.kd = PID_KD_DEFAULT;

    state->reference_temperature_mode = REFERENCE_TEMPERATURE_MODE_DEFAULT;
}
