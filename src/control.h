#ifndef __CONTROL_H__
#define __CONTROL_H__

#include "state.h"
#include "commands.h"

#define CONTROL_POLLING_S 1

void init_control(State *state, CommandArgs *commands);


#endif