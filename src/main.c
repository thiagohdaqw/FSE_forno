#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "pid.h"
#include "state.h"
#include "commands.h"
#include "user_interface.h"

static const char identifier[4] = { 0, 3, 7, 7 };
static State global_state;
static Uart uart;

void shutdown(int sig);

int main() {
    set_default_state(&global_state);
    signal(SIGINT, shutdown);

    init_uart(&uart, UART_DEVICE, ESP32_ADDRESS, APP_ADDRESS, identifier);

    init_commands(&global_state, &uart, identifier);
    // pause();
    run_user_interface(&global_state);
}

void shutdown(int sig) {
    printf("Finalizando aplicação...\n");
    close(uart.fd);
    exit(1);
}
