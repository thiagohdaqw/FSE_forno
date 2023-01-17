#ifndef __MODBUS_H__
#define __MODBUS_H__

#include <sys/time.h>

typedef struct {
    int fd;
    char dst;
    char src;
    struct timeval last_read;
} Uart;


int setup_uart(char *device, Uart *uart);
int receive_header(Uart *uart, char *buffer);
char *receive_message(Uart *uart, char *buffer, int size);
int send_message(Uart *uart, char code, char *data, int size);
#endif