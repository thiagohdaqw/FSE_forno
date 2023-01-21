#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "logging.h"
#include "state.h"

static FILE *log_file;
static State *global_state;

void run_logging(int sig);
void load_datetime(char *datetime, int size);

void init_logging(State *state) {
    log_file = fopen(LOG_FILE, "a");
    global_state = state;

    signal(SIGALRM, run_logging);
    alarm(LOG_ALARM_DELAY);
}

void run_logging(int sig) {
    char datetime[50];
    alarm(LOG_ALARM_DELAY);
    load_datetime(datetime, 50);
    fprintf(log_file, "%s,%.2f,%.2f,%.2f,%d\n", datetime, global_state->intern_temperature, global_state->extern_temperature,
            global_state->reference_temperature.value, global_state->pid.value);
    fflush(log_file);
}

void close_logging() {
    alarm(0);
    fflush(log_file);
    fclose(log_file);
}

void load_datetime(char *datetime, int size) {
    time_t t;
    time(&t);

    strftime(datetime, size, "%FT%TZ", localtime(&t));
}
