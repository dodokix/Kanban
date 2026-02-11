#ifndef CLIENT_CORE_H
#define CLIENT_CORE_H
#include "../../shared/protocol/protocol.h"


void send_hello();
void print_help();
void handle_stdin();
void handle_server_message(Message* msg);
void handle_peer_message(Message* msg);
void check_auction_result();



#endif