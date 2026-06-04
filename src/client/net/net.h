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

/*
    inizializza le strutture dati ed apre i socket di ascolto
*/
int client_setup(int myport);
/*
    chiude i socket aperti per una chiusura del processo pulita
*/
void net_cleanup();

/*
    funzioni per l'invio di messaggi
*/
int send_to_server(Message* msg);
int send_to_peer(Message* msg, int peer_port);
int broadcast_to_peers(Message* msg, int* peer_list, int n_peers);

/*
    funzioni per la gestione delle connessioni
*/
int connect_to_peer(int peer_port);
int accept_peer_connection();
void close_peer_connection(int peer_port);

/*
    funzioni per la ricezione di messaggi
*/
int receive_server_message(Message* msg);
int receive_message(Message* msg, int socket_fd);
/*
    ritorna il file descriptor del socket sapendo la porta di ascolto del peer
*/
int get_peer_socket(int peer_port);
/*
    aggiorna la struttura dati "peers" con la porta di ascolto del mittente
*/
void update_peer_port(int socket_fd, int port);

#endif