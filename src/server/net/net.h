#ifndef SERVER_NET_H
#define SERVER_NET_H

#define CMD_BUFF_SIZE 1024
#include <stdint.h>

typedef struct {
    int client_fd;
    uint16_t port;
    char buffer[CMD_BUFF_SIZE];
} ServerEvent;

int server_setup();
int get_command_from_net(ServerEvent*);

#endif