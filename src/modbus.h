#ifndef __MODBUS_H__
#define __MODBUS_H__

#include <sys/time.h>

#define BUFFER_SIZE 100

typedef struct {
    char items[BUFFER_SIZE];
    int cursor;
    int length;
} Buffer;

typedef struct {
    int fd;
    char dst;
    char src;
    struct timeval last_read;
    const char *identifier;
    Buffer buffer;
} Uart;


void init_uart(Uart *uart, char *device, char dst, char src, const char* identifier);
int receive_data(Uart *uart, char *buffer, int buffer_size);
int next_byte(Uart *uart, char *byte);
int receive_message(Uart *uart, char dst, char code, char sub_code, char *message, int size);
int send_message(Uart *uart, char code, char sub_code, char *data, int size);
#endif