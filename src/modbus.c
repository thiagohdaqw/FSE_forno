
#include "modbus.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>

#include "crc16.h"

#define READ_UART_MIN_US 100 * 1000

void init_uart(Uart *uart, char *device, char dst, char src, const char *identifier) {
    memset(uart, 0, sizeof(Uart));

    uart->fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);

    if (uart->fd == -1) {
        fprintf(stderr, "UART: Failed to open device %s\n", device);
        exit(1);
    }

    struct termios options;
    tcgetattr(uart->fd, &options);
    options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    tcflush(uart->fd, TCIFLUSH);
    tcsetattr(uart->fd, TCSANOW, &options);

    gettimeofday(&uart->last_read, NULL);
    uart->dst = dst;
    uart->src = src;
    uart->identifier = identifier;
}

int receive_data(Uart *uart, char *buffer, int buffer_size) {
    struct timeval tv;
    int bytes = 0;

    gettimeofday(&tv, NULL);
    if (tv.tv_usec - uart->last_read.tv_usec < READ_UART_MIN_US) {
        usleep(READ_UART_MIN_US - (tv.tv_usec - uart->last_read.tv_usec));
    }

    bytes = read(uart->fd, buffer, buffer_size);

    gettimeofday(&uart->last_read, NULL);
    return bytes;
}

int receive_safe_data(Uart *uart, char *buffer, int size) {
    int received = 0;
    int bytes = 0;

    while (received < size) {
        bytes = receive_data(uart, buffer + received, size - received);
        if (bytes <= 0) {
            return 0;
        }
        received += bytes;
    }
    return 1;
}

void load_buffer(Uart *uart) {
    Buffer *buffer = &uart->buffer;

    if (buffer->cursor >= buffer->length) {
        memset(buffer->items, 0, BUFFER_SIZE);
        buffer->length = receive_data(uart, buffer->items, BUFFER_SIZE);
        buffer->cursor = 0;
    }
}

int next_byte(Uart *uart, char *byte) {
    Buffer *buffer = &uart->buffer;

    load_buffer(uart);

    if (buffer->cursor < buffer->length) {
        *byte = buffer->items[buffer->cursor++];
        return 1;
    }

    return 0;
}

int receive_message(Uart *uart, char dst, char code, char sub_code, char *message, int size) {
    Buffer *buffer = &uart->buffer;
    int message_cursor = 0;
    while (buffer->cursor < buffer->length && message_cursor < size) {
        message[message_cursor++] = buffer->items[buffer->cursor++];
    }

    if (message_cursor < size || buffer->cursor + 2 > buffer->length) {
        int buffer_items = buffer->cursor - buffer->length;
        int length = size - message_cursor - 1 + 2 - buffer_items;
        for (int i = 0; i < buffer_items; i++, buffer->cursor++) {
            buffer->items[i] = buffer->items[buffer->cursor];
        }
        buffer->cursor = 0;
        buffer->length = buffer_items;
        if (!receive_safe_data(uart, buffer->items + buffer->length, length)) {
            return 0;
        }
        while (message_cursor < size) {
            message[message_cursor++] = buffer->items[buffer->cursor++];
        }
    }

    unsigned short crc = 0;
    crc = CRC16(crc, dst);
    crc = CRC16(crc, code);
    crc = CRC16(crc, sub_code);
    crc = calcula_CRC(crc, message, size);
    unsigned short crc_received = 0;
    memcpy(&crc_received, &buffer->items[buffer->cursor], 2);
    buffer->cursor += 2;

    if (crc != crc_received) {
        printf("CRC DIFERENTE\n");
        return 0;
        return 1;
    }
}

int send_message(Uart *uart, char code, char sub_code, char *data, int size) {
    int sended = 0;
    unsigned short crc = 0;

    sended += write(uart->fd, &uart->dst, 1);
    sended += write(uart->fd, &code, 1);
    sended += write(uart->fd, &sub_code, 1);
    sended += write(uart->fd, uart->identifier, 4);

    crc = CRC16(crc, uart->dst);
    crc = CRC16(crc, code);
    crc = CRC16(crc, sub_code);
    crc = calcula_CRC(crc, uart->identifier, 4);

    if (size > 0) {
        sended += write(uart->fd, data, size);
        crc = calcula_CRC(crc, data, size);
    }

    sended += write(uart->fd, &crc, 2);

    return sended == 7 + size + 2;
}
