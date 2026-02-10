#ifndef CLIENT_NET_H
#define CLIENT_NET_H

#include "protocol.h"

typedef struct {
    int port;
    int socket;
} PeerConnection;

extern int server_fd;
extern int listen_fd;
extern int my_port;
extern PeerConnection peers[MAX_USERS];
extern int num_peers;

int client_setup(int myport);
void check_ioevents();
int get_next_event();
int client_clenup();
int send_to_server(const Message* msg);

#endif