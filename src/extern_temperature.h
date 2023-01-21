#ifndef __EXTERN_TEMPERATURE_H__
#define __EXTERN_TEMPERATURE_H__

#include "state.h"
#include "commands.h"

#define EXTERN_TEMPERATURE_POLLING_SECONDS 1
#define EXTERN_TEMPERATURE_ADDRESS "/dev/i2c-1"

void init_extern_temperature(State *state);

#endif