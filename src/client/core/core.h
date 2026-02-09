#ifndef CLIENT_CORE_H
#define CLIENT_CORE_H


void send_hello();
void print_help();
void handle_stdin();
void handle_server_message(Message* msg);
void handle_peer_message(Message* msg, int peer_socket);



#endif