#ifndef __CONTROL_H__
#define __CONTROL_H__

#include "state.h"
#include "commands.h"

#define RESISTOR_PIN    4
#define FAN_PIN         5

#define FAN_MIN_PERCENT 40

#define CONTROL_POLLING_US 750 * 1000

#define PID_KP_DEFAULT 30.0
#define PID_KI_DEFAULT 0.2
#define PID_KD_DEFAULT 400.0

#define REFERENCE_TEMPERATURE_FILE_PATH "./curva_reflow.csv"

void stop_control();
void init_control(State *state);
void stop_file_mode(State *state);
void start_file_mode(State *state);

#endif