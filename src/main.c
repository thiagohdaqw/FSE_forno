#include<signal.h>
#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>

#include "modbus.h"

#define DATA_SIZE 5

#define REMOTE_ADDRESS 0x01
#define LOCAL_ADDRESS 0X00
#define DEVICE "/dev/serial0"

#define GET_CODE 0x23
#define GET_INTERN_TEMP 0xC1
#define GET_REFERENCE_TEMP 0xC2
#define GET_USER_COMMAND 0xC3

int uart = -1;

void handle_interruption(int sig);

int main() {
    char data[DATA_SIZE] = {0, 0, 3, 7, 7};
    char buffer[10] = {[0 ... 9] = 0};
    char *message;

    uart = setup_uart(DEVICE);

    signal(SIGINT, handle_interruption);

    char option = 1;

    while (option > 0) {
        scanf("%c", &option);

        switch (option)
        {
        case '1':
            data[0] = GET_INTERN_TEMP;
            enviar_mensagem(uart, REMOTE_ADDRESS, GET_CODE, data, DATA_SIZE);
            message = receber_mensagem(uart, LOCAL_ADDRESS, GET_CODE, buffer, 4);
            if (message) {
                printf("Temperatura interna: %.2f graus\n", *((float *) message));
            }
            break;
        case '2':
            data[0] = GET_REFERENCE_TEMP;
            enviar_mensagem(uart, REMOTE_ADDRESS, GET_CODE, data, DATA_SIZE);
            message = receber_mensagem(uart, LOCAL_ADDRESS, GET_CODE, buffer, 4);
            if (message) {
                printf("Temperatura de referencia: %.2f graus\n", *((float *) message));
            }
            break;
        case '3':
            data[0] = GET_USER_COMMAND;
            enviar_mensagem(uart, REMOTE_ADDRESS, GET_CODE, data, DATA_SIZE);
            message = receber_mensagem(uart, LOCAL_ADDRESS, GET_CODE, buffer, 4);
            if (message) {
                printf("Comando: %d\n", *((int *) message));
            }
            break;
        }
    }
    return 0;
}


void handle_interruption(int sig) {
    printf("Finalizando a aplicação devido a SIGINT...");
    close(uart);
    exit(1);
}
