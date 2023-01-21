#ifndef __LOGGING_H__
#define __LOGGING_H__

#include "state.h"

#define LOG_ALARM_DELAY 1
#define LOG_FILE "log.csv"

void init_logging(State *state);
void close_logging();

#endif