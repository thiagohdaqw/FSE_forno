#include "state.h"
#include "control.h"

void set_default_state(State *state) {
    state->extern_temperature = 0;
    state->intern_temperature = 0;
    state->reference_temperature.mode = REFERENCE_TEMPERATURE_MODE_DEFAULT;

    state->reference_temperature.value = 0;
    state->reference_temperature.is_debug = 0;

    state->fan_percent = 0;
    state->resistor_percent = 0;
    
    state->is_working = 0;
    state->is_heating = 0;
    
    state->pid.kp = PID_KP_DEFAULT;
    state->pid.ki = PID_KI_DEFAULT;
    state->pid.kd = PID_KD_DEFAULT;
}
