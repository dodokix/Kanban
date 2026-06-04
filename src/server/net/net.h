#ifndef SERVER_NET_H
#define SERVER_NET_H

#include <stdint.h>
#include "../../shared/core/core.h"
#include "../../shared/protocol/protocol.h"
#include "../../shared/constants/net_constants.h"

#define CMD_BUFF_SIZE 1024

// inizializza socket e strutture dati della rete
int server_setup();
// funzione chiamata nel ciclo del main per gestire la rete
int check_net();
// chiusura rete
void close_net();
// ricezione messaggi 
int get_message_from_net(Message* msg);
// invio messaggi
int send_to_client(Message* msg, int client_fd);

#endif