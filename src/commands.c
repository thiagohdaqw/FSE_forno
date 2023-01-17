#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "commands.h"
#include "modbus.h"

#define COMMAND_POLLING_US 500 * 1000


CommandArgs *init_commands() {

}

void watch_commands(void *commands_args) {
  char buffer[COMMAND_MESSAGE_MAX_SIZE];
  char *message;
  Command *command;
  CommandArgs *commands = (CommandArgs *)commands_args;

  while (1) {
    memset(buffer, 0, 10);

    if (receive_header(commands->uart, buffer)) {
      for (int i=0; i < commands->commands_size; i++) {
        command = &commands->commands[i];
        if (command->code == buffer[1] && command->sub_code == buffer[2]) {
          message = receive_message(commands->uart, buffer, command->message_size);
          if (message) {
            commands->callbacks[i](command, message, commands->states);
          }
        }
      }
    }

    usleep(COMMAND_POLLING_US);    
  }
}
