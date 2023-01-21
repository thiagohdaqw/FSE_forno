#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>

#include "commands.h"
#include "modbus.h"
#include "state.h"
#include <pid.h>

#define MESSAGE_BUFFER_MAX 10

static CommandArgs commands;

void *send_commands(void *args);
void *read_commands(void *args);
void send_default_state_command(State *state, char working);
void fill_commands();

CommandArgs *init_commands(State *state, Uart *uart, const char *identifier) {
    pthread_t tid;

    commands.uart = uart;
    commands.state = state;
    fill_commands();

    pthread_create(&tid, NULL, send_commands, NULL);
    pthread_create(&tid, NULL, read_commands, NULL);
    usleep(500*1000);
    send_default_state_command(state, 0);
    return &commands;
}

void send_command(int command, State *state) {
    // printf("Sending: %d\n", command);
    commands.send_funcs[command](&commands.commands[command], state, commands.uart);
}

void *send_commands(void *args) {
    while (1) {
        for (int i = 0; i < COMMAND_TO_SEND_SIZE; i++) {
            send_command(i, commands.state);
        }
        usleep(COMMAND_SEND_POLLING_US);
    }
}

void *read_commands(void *commands_args) {
    char message[MESSAGE_BUFFER_MAX];
    char dst, code, sub_code;
    Command *command;
    Uart *uart = commands.uart;

    while (1) {
        while (next_byte(uart, &dst)) {
            if (dst != uart->src) {
                continue;
            }

            if (!next_byte(uart, &code)) {
                break;
            }

            if (code != 0x23 && code != 0x16) {
                continue;
            }

            if (!next_byte(uart, &sub_code)) {
                break;
            }

            for (int i = 0; i < commands.commands_size; i++) {
                command = &commands.commands[i];
                if (command->code == code && command->sub_code == sub_code) {
                    memset(message, 0, MESSAGE_BUFFER_MAX);
                    //printf("%d %d %d\n", dst, code, sub_code);
                    if (receive_message(commands.uart, dst, code, sub_code, message, command->message_size)) {
                        commands.read_funcs[i](command, message, commands.state);
                    } else {
                        send_command(i, commands.state);
                    }
                    break;
                }
            }
        }

        usleep(COMMAND_READ_POLLING_US);
    }
}

void read_intern_temperature_read_command(Command *command, char *message, State *state) {
    memcpy(&state->intern_temperature, message, 4);
    //printf("Intern temperature: %f\n", state->intern_temperature);
}

void read_reference_temperature_read_command(Command *command, char *message, State *state) {
    memcpy(&state->reference_temperature, message, 4);
    //printf("Reference temperature: %f\n", state->reference_temperature);
}

void send_default_state_command(State *state, char working) {
    set_default_state(state);
    state->is_working = working;

    send_command(COMMAND_SEND_WORKING_STATUS, state);
    send_command(COMMAND_SEND_HEATING_STATUS, state);
    send_command(COMMAND_SEND_REFERENCE_TEMPERATURE_MODE, state);
}

void read_user_command(Command *command, char *message, State *state) {
    char previous_working_status = state->is_working;
    
    int user_command = -1;
    memcpy(&user_command, message, 4);

    // printf("Command: %d\n", user_command);

    switch (user_command) {
    case USER_COMMAND_ON:
        send_default_state_command(state, 1);
        
        if (previous_working_status == 0) {
            for (int i=0; i < WORKING_EVENT_SIZE; i++) {
                sem_post(&state->working_event);
            }
        }
        break;
    case USER_COMMAND_OFF:
        send_default_state_command(state, 0);
        break;
    case USER_COMMAND_HEATING_ON:
        state->is_heating = state->is_working;
        send_command(COMMAND_SEND_HEATING_STATUS, state);
        if (state->is_heating) {
            sem_post(&state->heating_event);
        }
        break;
    case USER_COMMAND_HEATING_OFF:
        state->is_heating = 0;
        send_command(COMMAND_SEND_HEATING_STATUS, state);
        break;
    case USER_COMMAND_MENU:
        if (!state->reference_temperature_debug_mode) {
            state->reference_temperature_mode = state->reference_temperature_mode == REFERENCE_TEMPERATURE_MODE_UART
                                                    ? REFERENCE_TEMPERATURE_MODE_FILE
                                                    : REFERENCE_TEMPERATURE_MODE_UART;
            send_command(COMMAND_SEND_REFERENCE_TEMPERATURE_MODE, state);
        }
        break;
    }
}

void send_control_command(Command *command, State *state, Uart *uart) {
    send_message(uart, command->code, command->sub_code, (char *)&state->pid.value, 4);
}

void send_reference_temperature_command(Command *command, State *state, Uart *uart) {
    send_message(uart, command->code, command->sub_code, (char *)&state->reference_temperature, 4);
}

void send_working_status_command(Command *command, State *state, Uart *uart) {
    send_message(uart, command->code, command->sub_code, (char *)&state->is_working, 1);
}

void send_reference_temperature_mode_command(Command *command, State *state, Uart *uart) {
    char reference_temperature_mode = state->reference_temperature_mode != REFERENCE_TEMPERATURE_MODE_UART;
    send_message(uart, command->code, command->sub_code, &reference_temperature_mode, 1);
}

void send_heating_status_command(Command *command, State *state, Uart *uart) {
    send_message(uart, command->code, command->sub_code, (char *)&state->is_heating, 1);
}

void send_extern_temperature_command(Command *command, State *state, Uart *uart) {
    send_message(uart, command->code, command->sub_code, (char *)&state->extern_temperature, 4);
}

void read_working_status_command(Command *command, char *message, State *state) {
    int working = 0;
    memcpy(&working, message, 4);
    if (working != state->is_working) {
        send_command(COMMAND_SEND_WORKING_STATUS, state);
    }
}

void read_reference_temperature_mode_command(Command *command, char *message, State *state) {
    int reference_temperature_mode = 0;
    memcpy(&reference_temperature_mode, message, 4);
    if (reference_temperature_mode != state->reference_temperature_mode) {
        send_command(COMMAND_SEND_REFERENCE_TEMPERATURE_MODE, state);
    }
}

void read_heating_status_command(Command *command, char *message, State *state) {
    int heating_status = 0;
    memcpy(&heating_status, message, 4);
    if (heating_status != state->is_heating) {
        send_command(COMMAND_SEND_HEATING_STATUS, state);
    }
}

void read_empty_command(Command *command, char *message, State *state) {}

void send_empty_command(Command *command, State *state, Uart *uart) {
    send_message(uart, command->code, command->sub_code, NULL, 0);
}

void send_empty_when_working_command(Command *command, State *state, Uart *uart) {
    if (state->is_working) {
        send_empty_command(command, state, uart);
    }
}

void fill_commands() {
    commands.commands_size = 9;
    commands.commands = malloc(commands.commands_size * sizeof(Command));
    commands.read_funcs = malloc(commands.commands_size * sizeof(ReadCommandFunc));
    commands.send_funcs = malloc(commands.commands_size * sizeof(SendCommandFunc));

    commands.commands[COMMAND_READ_INTERN_TEMPERATURE].code = 0x23;
    commands.commands[COMMAND_READ_INTERN_TEMPERATURE].sub_code = 0xC1;
    commands.commands[COMMAND_READ_INTERN_TEMPERATURE].message_size = 4;
    commands.read_funcs[COMMAND_READ_INTERN_TEMPERATURE] = read_intern_temperature_read_command;
    commands.send_funcs[COMMAND_READ_INTERN_TEMPERATURE] = send_empty_when_working_command;

    commands.commands[COMMAND_READ_REFERENCE_TEMPERATURE].code = 0x23;
    commands.commands[COMMAND_READ_REFERENCE_TEMPERATURE].sub_code = 0xC2;
    commands.commands[COMMAND_READ_REFERENCE_TEMPERATURE].message_size = 4;
    commands.read_funcs[COMMAND_READ_REFERENCE_TEMPERATURE] = read_reference_temperature_read_command;
    commands.send_funcs[COMMAND_READ_REFERENCE_TEMPERATURE] = send_empty_when_working_command;

    commands.commands[COMMAND_READ_USER_COMMANDS].code = 0x23;
    commands.commands[COMMAND_READ_USER_COMMANDS].sub_code = 0xC3;
    commands.commands[COMMAND_READ_USER_COMMANDS].message_size = 4;
    commands.read_funcs[COMMAND_READ_USER_COMMANDS] = read_user_command;
    commands.send_funcs[COMMAND_READ_USER_COMMANDS] = send_empty_command;

    commands.commands[COMMAND_SEND_CONTROL].code = 0x16;
    commands.commands[COMMAND_SEND_CONTROL].sub_code = 0xD1;
    commands.commands[COMMAND_SEND_CONTROL].message_size = 0;
    commands.read_funcs[COMMAND_SEND_CONTROL] = read_empty_command;
    commands.send_funcs[COMMAND_SEND_CONTROL] = send_control_command;

    commands.commands[COMMAND_SEND_REFERENCE_TEMPERATURE].code = 0x16;
    commands.commands[COMMAND_SEND_REFERENCE_TEMPERATURE].sub_code = 0xD2;
    commands.commands[COMMAND_SEND_REFERENCE_TEMPERATURE].message_size = 0;
    commands.read_funcs[COMMAND_SEND_REFERENCE_TEMPERATURE] = read_empty_command;
    commands.send_funcs[COMMAND_SEND_REFERENCE_TEMPERATURE] = send_reference_temperature_command;

    commands.commands[COMMAND_SEND_WORKING_STATUS].code = 0x16;
    commands.commands[COMMAND_SEND_WORKING_STATUS].sub_code = 0xD3;
    commands.commands[COMMAND_SEND_WORKING_STATUS].message_size = 4;
    commands.read_funcs[COMMAND_SEND_WORKING_STATUS] = read_working_status_command;
    commands.send_funcs[COMMAND_SEND_WORKING_STATUS] = send_working_status_command;

    commands.commands[COMMAND_SEND_REFERENCE_TEMPERATURE_MODE].code = 0x16;
    commands.commands[COMMAND_SEND_REFERENCE_TEMPERATURE_MODE].sub_code = 0xD4;
    commands.commands[COMMAND_SEND_REFERENCE_TEMPERATURE_MODE].message_size = 4;
    commands.read_funcs[COMMAND_SEND_REFERENCE_TEMPERATURE_MODE] = read_reference_temperature_mode_command;
    commands.send_funcs[COMMAND_SEND_REFERENCE_TEMPERATURE_MODE] = send_reference_temperature_mode_command;

    commands.commands[COMMAND_SEND_HEATING_STATUS].code = 0x16;
    commands.commands[COMMAND_SEND_HEATING_STATUS].sub_code = 0xD5;
    commands.commands[COMMAND_SEND_HEATING_STATUS].message_size = 4;
    commands.read_funcs[COMMAND_SEND_HEATING_STATUS] = read_heating_status_command;
    commands.send_funcs[COMMAND_SEND_HEATING_STATUS] = send_heating_status_command;

    commands.commands[COMMAND_SEND_EXTERN_TEMPERATURE].code = 0x16;
    commands.commands[COMMAND_SEND_EXTERN_TEMPERATURE].sub_code = 0xD6;
    commands.commands[COMMAND_SEND_EXTERN_TEMPERATURE].message_size = 4;
    commands.read_funcs[COMMAND_SEND_EXTERN_TEMPERATURE] = read_empty_command;
    commands.send_funcs[COMMAND_SEND_EXTERN_TEMPERATURE] = send_extern_temperature_command;
}