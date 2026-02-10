#ifndef SERVER_NET_H
#define SERVER_NET_H

#define CMD_BUFF_SIZE 1024
#include <stdint.h>
#include "../../shared/core/core.h"
#include "../../shared/protocol/protocol.h"



int server_setup();
int check_net();
void close_net();
/*
get_command_from_net estrae una riga inviata al server.
*/
int get_command_from_net(Message* msg);

#endif