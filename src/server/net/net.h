#ifndef SERVER_NET_H
#define SERVER_NET_H

#include <stdint.h>
#include "../../shared/core/core.h"
#include "../../shared/protocol/protocol.h"
#include "../../shared/constants/net_constants.h"

#define CMD_BUFF_SIZE 1024


int server_setup();
int check_net();
void close_net();
int get_message_from_net(Message* msg);
int send_to_client(Message* msg, int client_fd);

#endif