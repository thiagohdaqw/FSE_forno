
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

int setup_uart(char *device, Uart *uart) {
  uart->fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);

  if (uart->fd == -1) {
    return 0;
  }

  struct termios options;
  tcgetattr(uart->fd, &options);
  options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
  options.c_iflag = IGNPAR;
  options.c_oflag = 0;
  options.c_lflag = 0;
  tcflush(uart->fd, TCIFLUSH);
  tcsetattr(uart->fd, TCSANOW, &options);
  return 1;
}

int receive_data(Uart *uart, char *data, int size) {
  int received = 0;
  int bytes = 0;
  struct timeval tv;

  while (received < size) {
    gettimeofday(&tv, NULL);
    if (tv.tv_usec - uart->last_read.tv_usec < READ_UART_MIN_US) {
      usleep(READ_UART_MIN_US - (tv.tv_usec - uart->last_read.tv_usec));
    }

    bytes = read(uart->fd, data + received, size - received);

    gettimeofday(&uart->last_read, NULL);
    if (bytes <= 0) {
      return 0;
    }
    received += bytes;
  }
  return 1;
}

int receive_header(Uart *uart, char *buffer) {
  if (!receive_data(uart->fd, buffer, 3) || uart->src != buffer[0]) {
    return 0;
  }
  return 1;
}

char *receive_message(Uart *uart, char *buffer, int size) {
  if (!receive_data(uart->fd, buffer + 3, size + 2)) {
    return 0;
  }

  unsigned short crc = calcula_CRC(0, buffer, 3 + size);
  unsigned short crc_received = 0;
  memcpy(&crc_received, buffer + 3 + size, 2);

  if (crc != crc_received) {
    return NULL;
  }
  return buffer + 3;
}

int send_message(Uart *uart, char code, char *data, int size) {
  int sended = 0;
  short crc = 0;

  sended += write(uart->fd, &uart->dst, 1);
  sended += write(uart->fd, &code, 1);
  sended += write(uart->fd, data, size);

  crc = CRC16(crc, uart->dst);
  crc = CRC16(crc, code);
  crc = calcula_CRC(crc, data, size);

  sended += write(uart->fd, &crc, 2);

  return sended == 1 + 1 + size + 2
}
