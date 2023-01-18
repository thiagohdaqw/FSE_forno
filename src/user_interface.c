#include <stdio.h>
#include <stdlib.h>

#include "state.h"
#include "user_interface.h"

char read_main_menu();
void run_reference_temperature_interface(State *state);
void run_pid_interface(State *state);
char read_reference_temperature_menu(State *state);
void clear_console();
void repeat_caracter(char caracter, int length, char end);

void run_user_interface(State *state) {
    while (1) {
        switch (read_main_menu()) {
        case '1':
            run_reference_temperature_interface(state);
            break;
        case '2':
            run_pid_interface(state);
        }
    }
}

char read_main_menu() {
    char option;

    clear_console();
    repeat_caracter('=', 100, '\n');
    printf("--- - Menu\n");
    printf("[1] - Temperatura de Referencia (TR)\n");
    printf("[2] - Parametros PID\n");
    repeat_caracter('=', 100, '\n');
    scanf(" %c", &option);
    return option;
}

void run_reference_temperature_interface(State *state) {
    while (1) {
        switch (read_reference_temperature_menu(state)) {
        case '1':
            state->reference_temperature_mode =
                REFERENCE_TEMPERATURE_MODE_DEBUG;
            break;
        case '2':
            state->reference_temperature_mode = REFERENCE_TEMPERATURE_MODE_UART;
            break;
        case '3':
            state->reference_temperature_mode = REFERENCE_TEMPERATURE_MODE_FILE;
            break;
        case '0':
            return;
        }
    }
}
void run_pid_interface(State *state) {
    clear_console();
    repeat_caracter('=', 100, '\n');
    printf("--- - Parametros PID\n");
    repeat_caracter('=', 100, '\n');

    printf("Kp: ");
    scanf(" %f", &state->pid.kp);
    printf("Ki: ");
    scanf(" %f", &state->pid.ki);
    printf("Kd: ");
    scanf(" %f", &state->pid.kd);
}

char read_reference_temperature_menu(State *state) {
    char option;

    clear_console();
    repeat_caracter('=', 100, '\n');
    printf("--- - Temperatura de Referencia (TR)\n");
    printf("[%c] - Modo Debug\n",
           state->reference_temperature_mode == REFERENCE_TEMPERATURE_MODE_DEBUG
               ? '*'
               : '1');
    printf("[%c] - Modo Dashboard UART\n",
           state->reference_temperature_mode == REFERENCE_TEMPERATURE_MODE_UART
               ? '*'
               : '2');
    printf("[%c] - Modo Pre-definido em arquivo\n",
           state->reference_temperature_mode == REFERENCE_TEMPERATURE_MODE_FILE
               ? '*'
               : '3');
    printf("[0] - Voltar\n");
    repeat_caracter('=', 100, '\n');
    scanf(" %c", &option);
    return option;
}

void clear_console() { repeat_caracter('\n', 100, '\n'); }

void repeat_caracter(char caracter, int length, char end) {
    for (int i = 0; i < length; i++) {
        printf("%c", caracter);
    }
    printf("%c", end);
}
