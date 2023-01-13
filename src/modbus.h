#ifndef __MODBUS_H__
#define __MODBUS_H__

int setup_uart(char *device);

char *receber_mensagem(int fd, char dst, char code, char *data, int size);
char *receber_mensagem_dinamica(int fd, char dst, char code, char *data);
int enviar_mensagem(int fd, char dst, char code, char *data, int size);

#endif