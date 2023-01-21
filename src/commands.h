#ifndef __COMMANDS_H__
#define __COMMANDS_H__

#include "modbus.h"
#include "state.h"

#define BUFFER_LENGTH 100
#define COMMAND_READ_POLLING_US 500 * 1000
#define COMMAND_SEND_POLLING_US 1000 * 1000

#define COMMAND_READ_INTERN_TEMPERATURE 0
#define COMMAND_READ_REFERENCE_TEMPERATURE 1
#define COMMAND_READ_USER_COMMANDS 2
#define COMMAND_SEND_CONTROL 3
#define COMMAND_SEND_REFERENCE_TEMPERATURE 4
#define COMMAND_SEND_WORKING_STATUS 5
#define COMMAND_SEND_REFERENCE_TEMPERATURE_MODE 6
#define COMMAND_SEND_HEATING_STATUS 7
#define COMMAND_SEND_EXTERN_TEMPERATURE 8

#define COMMAND_TO_SEND_SIZE 3

#define USER_COMMAND_ON 0xA1
#define USER_COMMAND_OFF 0xA2
#define USER_COMMAND_HEATING_ON 0xA3
#define USER_COMMAND_HEATING_OFF 0xA4
#define USER_COMMAND_MENU 0xA5

typedef struct {
    char code;
    char sub_code;
    int message_size;
    int send_polling_us;
} Command;

typedef void (*ReadCommandFunc)(Command *command, char *message, State *state);
typedef void (*SendCommandFunc)(Command *command, State *state, Uart *uart);

typedef struct {
    Uart *uart;
    State *state;
    int commands_size;
    Command *commands;
    ReadCommandFunc *read_funcs;
    SendCommandFunc *send_funcs;
} CommandArgs;

CommandArgs *init_commands(State *state, Uart *uart, const char *identifier);
void send_command(int command, State *state);
void send_default_state_command(State *state, char working);

#endif