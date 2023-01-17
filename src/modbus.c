
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

#include <crc16.h>
#include "modbus.h"

int setup_uart(char *device) {
  int uart_fd = -1;

  uart_fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);

  if (uart_fd == -1) {
    printf("Erro - Não foi possível iniciar a UART.\n");
    exit(1);
  } else {
    printf("UART inicializada!\n");
  }

  struct termios options;
  tcgetattr(uart_fd, &options);
  options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
  options.c_iflag = IGNPAR;
  options.c_oflag = 0;
  options.c_lflag = 0;
  tcflush(uart_fd, TCIFLUSH);
  tcsetattr(uart_fd, TCSANOW, &options);
  return uart_fd;
}

int receber_dados(int fd, char *data, int size) {
  int received = 0;
  int bytes = 0;

  while (received < size) {
    usleep(200000);
    bytes = read(fd, data + received, size - received);

    if (bytes <= 0) {
      printf("Erro no recebimento dos dados, recebido: %d, esperado: %d\n",
             received, size);
      return 0;
    }
    received += bytes;
  }
  return 1;
}

int cabecalho_valido(char dst, char code, char *data) {
  if (dst != data[0]) {
    printf("Destinatario diferente. Esperado %d, recebido %d\n", dst, data[0]);
    return 0;
  }
  if (code != data[1]) {
    printf("Codigo da funcao diferente do esperado. Esperada %d, recebido %d\n",
           code, data[1]);
    return 0;
  }
  return 1;
}

char *receber_mensagem(int fd, char dst, char code, char *data, int size) {
  if (!receber_dados(fd, data, 3 + size + 2)) {
    printf("Erro no recebimento da mensagem\n");
    return NULL;
  }

  if (!cabecalho_valido(dst, code, data)) {
    return NULL;
  }

  unsigned short crc = calcula_CRC(0, data, 3 + size);
  unsigned short crc_received = *((short *)(data + 3 + size));

  if (crc != crc_received) {
    printf("O CRC calculado %d eh diferente do recebido %d\n", crc,
           crc_received);
    return NULL;
  }
  return data + 3;
}

char *receber_mensagem_dinamica(int fd, char dst, char code, char *data) {
  if (!receber_dados(fd, data, 4) ||
      !receber_dados(fd, data + 4, data[3] + 2)) {
    printf("Erro no recebimento da mensagem\n");
    return NULL;
  }

  if (!cabecalho_valido(dst, code, data)) {
    return NULL;
  }

  short crc = calcula_CRC(0, data, 4 + data[3]);
  short crc_received = *((short *)(data + 4 + data[3]));

  printf("%d\n", data[4+data[3]]);
  printf("%d\n", data[4+data[3]+1]);

  if (crc != crc_received) {
    printf("O CRC calculado %d eh diferente do recebido %d\n", crc,
           crc_received);
    return NULL;
  }
  data[4 + data[3]] = 0;
  return data + 4;
}

int enviar_mensagem(int fd, char dst, char code, char *data,
                    int size) {
  int sended = 0;
  short crc = 0;

  sended += write(fd, &dst, 1);
  sended += write(fd, &code, 1);
  sended += write(fd, data, size);

  crc = CRC16(crc, dst);
  crc = CRC16(crc, code);
  crc = calcula_CRC(crc, data, size);

  sended += write(fd, &crc, 2);

  printf("Enviado para %X, comando %X, %d bytes\n", dst, code, sended);
  if (sended != 1 + 1 + size + 2) {
    printf("Erro no envio da mensagem. Enviado %d, esperado %d\n", sended,
           1 + 1 + size + 2);
    return 0;
  }
  return 1;
}
