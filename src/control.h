#ifndef __CONTROL_H__
#define __CONTROL_H__

#include "state.h"
#include "commands.h"

#define CONTROL_POLLING_SECONDS 1

#define PID_KP_DEFAULT 30.0
#define PID_KI_DEFAULT 0.2
#define PID_KD_DEFAULT 400.0

void init_control(State *state, CommandArgs *commands);

#endif