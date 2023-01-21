#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>

#include "pid.h"
#include "state.h"
#include "control.h"
#include "commands.h"
#include "user_interface.h"
#include "extern_temperature.h"

static const char identifier[4] = { 0, 3, 7, 7 };
static State global_state;
static Uart uart;

void shutdown(int sig);

int main() {
    set_default_state(&global_state);
    sem_init(&global_state.working_event, 0, 0);
    signal(SIGINT, shutdown);

    init_uart(&uart, UART_DEVICE, ESP32_ADDRESS, APP_ADDRESS, identifier);
    CommandArgs *commands = init_commands(&global_state, &uart, identifier);
    init_control(&global_state, commands);
    init_extern_temperature(&global_state, commands);

    run_user_interface(&global_state);
}

void shutdown(int sig) {
    printf("Finalizando aplicação...\n");
    close(uart.fd);
    exit(1);
}
