#ifndef CLIENT_NET_H
#define CLIENT_NET_H

#include "../../shared/protocol/protocol.h"
#include "../../shared/constants/net_constants.h"

typedef struct {
    int port;
    int socket;
} PeerConnection;

extern int server_fd;
extern int listen_fd;
extern int my_port;
extern PeerConnection peers[MAX_PEER_CONNECTIONS];
extern int num_peers;
extern int pending_close_socket;

int client_setup(int myport);
void net_cleanup();

int send_to_server(Message* msg);
int send_to_peer(Message* msg, int peer_port);
int broadcast_to_peers(Message* msg, int* peer_list, int n_peers);

int connect_to_peer(int peer_port);
int accept_peer_connection();
void close_peer_connection(int peer_port);

int receive_server_message(Message* msg);
int receive_message(Message* msg, int socket_fd);
int get_peer_socket(int peer_port);
void update_peer_port(int socket_fd, int port);

#endif