#ifndef __COMMANDS_H__
#define __COMMANDS_H__

#include "modbus.h"

#define COMMAND_MESSAGE_MAX_SIZE 10

typedef struct
{
    char code;
    char sub_code;
    int message_size;
    CommandCallback callback;
} Command;

typedef void (*CommandCallback)(Command *command, char *message);

typedef struct
{
    Uart *uart;
    Command *commands;
    int commands_size;
    char identifier[4];
} CommandArgs;

#endif