#ifndef __COMMANDS_H__
#define __COMMANDS_H__

#include "modbus.h"
#include "defs.h"

#define COMMAND_MESSAGE_MAX_SIZE 10

typedef struct
{
    char code;
    char sub_code;
    int message_size;
} Command;

typedef void (*CommandCallback)(Command *command, char *message, States *states);

typedef struct
{
    Uart *uart;
    char identifier[4];
    States *states;
    int commands_size;
    Command *commands;
    CommandCallback *callbacks;
} CommandArgs;

#endif