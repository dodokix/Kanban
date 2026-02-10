#ifndef SERVER_NET_H
#define SERVER_NET_H

#define CMD_BUFF_SIZE 1024
#include <stdint.h>
#include "../../shared/core/core.h"



int server_setup();
int check_net();
void close_net();
/*
get_command_from_net estrae una riga inviata al server.
*/
int get_command_from_net(Message* msg);

int send_to_client(int client_fd, const char* buffer, int len);

#endif