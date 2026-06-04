#include "net.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

struct sockaddr_in usr_addr;

int server_fd = -1;
int listen_fd = -1;
int my_port = 0;
PeerConnection peers[MAX_PEER_CONNECTIONS];
int num_peers = 0;

static char srv_buf[CMD_BUFF_SIZE * 4];
static int  srv_buf_len = 0;

/*
==========================================================================================
    SETUP
==========================================================================================
*/

static int setup_server_socket(){
    struct sockaddr_in server_addr;

    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("Errore nella creazione del socket\n");
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);    

    if(inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0){
        perror("[ERROR]:Indirizzo server non valido\n");
        return -1;
    }

    if(connect(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        perror("[ERROR]:Connessione fallita\n");
        return -1;
    }

    printf("[NET] Connesso alla lavagna sulla porta %d\n", SERVER_PORT);
    return 0;
}

int setup_p2p_socket(int myport){
    struct sockaddr_in client_addr;

    if((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("[NET] socket P2P");
        return -1;
    }

    int opt = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("[NET] setsockopt");
    }

    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = INADDR_ANY;
    client_addr.sin_port = htons(myport);

    if(bind(listen_fd, (struct sockaddr*)&client_addr, sizeof(client_addr)) < 0){
        perror("[NET] bind P2P");
        return -1;
    }

    if(listen(listen_fd, BACKLOG) < 0){
        perror("[NET] listen P2P");
        return -1;
    }
    printf("[NET] In ascolto sulla porta %d per connessioni P2P\n", myport);
    return 0;
}

int client_setup(int port){
    my_port = port;
    num_peers = 0;

    for(int i = 0; i < MAX_PEER_CONNECTIONS; ++i){
        peers[i].port = 0;
        peers[i].socket = -1;
    }
    
    if(setup_server_socket() != 0){
        printf("[ERROR]: connessione al server fallita\n");
        return(-1);
    }

    if(setup_p2p_socket(port) != 0){
        printf("[ERROR]: fallita inizializzazione del socket di ascolto\n");
        return(-1);
    }

    return 0;
}

/*
==========================================================================================
    INVIO MESSAGGI
==========================================================================================
*/
int send_to_server(Message* msg) {
    char buffer[CMD_BUFF_SIZE];
    int len;
    
    len = serialize_message(msg, buffer, CMD_BUFF_SIZE);
    if(len < 0) {
        printf("[ERROR] Errore nella serializzazione del messaggio\n");
        return -1;
    }
    
    if(send(server_fd, buffer, len, 0) < 0) {
        perror("[NET] send al server");
        return -1;
    }

    return 0;
}

int send_to_peer(Message* msg, int peer_port) {
    char buffer[CMD_BUFF_SIZE];
    int peer_socket;
    
    peer_socket = get_peer_socket(peer_port);
        
    if(peer_socket < 0) {
        peer_socket = connect_to_peer(peer_port);
        if(peer_socket < 0) {
            printf("[ERROR] Impossibile connettersi al peer %d\n", peer_port);
            return -1;
        }
    }
    
    int len = serialize_message(msg, buffer, CMD_BUFF_SIZE);
    if(len < 0) {
        printf("[ERROR] Errore nella serializzazione del messaggio\n");
        return -1;
    }
    
    if(send(peer_socket, buffer, len, 0) < 0) {
        perror("[NET] errore nella send to peer\n");
        return -1;
    }

    return 0;
}

/*
==========================================================================================
    GESTIONE CONNESSIONI
==========================================================================================
*/
int connect_to_peer(int peer_port) {

    if(get_peer_socket(peer_port) >= 0) {
        return get_peer_socket(peer_port);
    }

    if(num_peers >= MAX_PEER_CONNECTIONS){
        printf("[WARN] Array peer pieno, impossibile connettersi a %d\n", peer_port);
        return -1;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0) {
        perror("[NET] socket peer");
        return -1;
    }
    
    struct sockaddr_in peer_addr;
    memset(&peer_addr, 0, sizeof(peer_addr));
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_port = htons(peer_port);
    
    if(inet_pton(AF_INET, "127.0.0.1", &peer_addr.sin_addr) <= 0) {
        perror("[NET] inet_pton");
        close(sock);
        return -1;
    }
    
    if(connect(sock, (struct sockaddr*)&peer_addr, sizeof(peer_addr)) < 0) {
        perror("[NET] connect");
        close(sock);
        return -1;
    }

    peers[num_peers].port = peer_port;
    peers[num_peers].socket = sock;
    num_peers++;
    printf("[NET] Connesso al peer %d\n", peer_port);
    
    return sock;
}

int accept_peer_connection() {
    struct sockaddr_in peer_addr;
    socklen_t addr_len = sizeof(peer_addr);
    
    int new_socket;
    new_socket = accept(listen_fd, (struct sockaddr*)&peer_addr, &addr_len);
    if(new_socket < 0) {
        perror("[NET] accept");
        return -1;
    }

    if(num_peers >= MAX_PEER_CONNECTIONS){
        printf("[WARN] Troppi peer, connessione rifiutata\n");
        close(new_socket);
        return -1;
    }

    /* salva la porta =0, verra' aggiornata con la porta reale quando arrivera' il primo messaggio. 
        che contiene sender_port.
    */
    peers[num_peers].port = 0;
    peers[num_peers].socket = new_socket;
    num_peers++;

    printf("[NET] Connessione P2P accettata (socket %d)\n", new_socket);
    
    return new_socket;
}

/*
==========================================================================================
    RICEZIONE MESSAGGI
==========================================================================================
*/
int receive_server_message(Message* msg) {
    char *nl = memchr(srv_buf, '\n', srv_buf_len);
    if (!nl) {
        int space = (int)sizeof(srv_buf) - 1 - srv_buf_len;
        if (space > 0) {
            int n = recv(server_fd, srv_buf + srv_buf_len, space, MSG_DONTWAIT);
            if (n > 0) {
                srv_buf_len += n;
                srv_buf[srv_buf_len] = '\0';
            } else if (n == 0) {
                return -1; // connessione chiusa
            }
            // n < 0 con EAGAIN: nessun dato nuovo, si usa solo il buffer
        }
        nl = memchr(srv_buf, '\n', srv_buf_len);
        if (!nl) return 0; // nessun messaggio completo
    }

    *nl = '\0';
    int rc = deserialize_message(srv_buf, msg);
    int msg_len = (int)(nl - srv_buf) + 1;
    srv_buf_len -= msg_len;
    memmove(srv_buf, nl + 1, srv_buf_len);
    srv_buf[srv_buf_len] = '\0';

    return rc == 0 ? 1 : -1;
}

int receive_message(Message* msg, int socket_fd) {
    char buffer[CMD_BUFF_SIZE];
    
    int bytes_read = recv(socket_fd, buffer, CMD_BUFF_SIZE - 1, 0);
    if(bytes_read < 0) {
        perror("[NET] recv");
        return -1;
    }
    
    if(bytes_read == 0) {
        // connessione chiusa
        return 0;
    }
    
    buffer[bytes_read] = '\0';
    
    if(deserialize_message(buffer, msg) < 0) {
        printf("[ERROR] Errore nella deserializzazione del messaggio\n");
        return -1;
    }
    
    return bytes_read;
}

/*
==========================================================================================
    CHIUSURA
==========================================================================================
*/
void close_peer_connection(int peer_port) {
    for(int i = 0; i < num_peers; i++) {
        if(peers[i].port == peer_port) {
            close(peers[i].socket);
            for(int j = i; j < num_peers - 1; j++) {
                peers[j] = peers[j + 1];
            }
            num_peers--;
            peers[num_peers].port = 0;
            peers[num_peers].socket = -1;
            printf("[NET] Connessione con peer %d chiusa\n", peer_port);
            
            return;
        }
    }
}

void net_cleanup() {
    if(server_fd >= 0) {
        close(server_fd);
    }
    
    if(listen_fd >= 0) {
        close(listen_fd);
    }
    
    for(int i = 0; i < num_peers; i++) {
        if(peers[i].socket >= 0) {
            close(peers[i].socket);
        }
    }
}

/*
==========================================================================================
    MISC
==========================================================================================
*/
int get_peer_socket(int peer_port) {
    for(int i = 0; i < num_peers; i++) {
        if(peers[i].port == peer_port) {
            return peers[i].socket;
        }
    }
    return -1;
}

void update_peer_port(int socket_fd, int port) {
    for(int i = 0; i < num_peers; i++) {
        if(peers[i].socket == socket_fd && peers[i].port == 0) {
            peers[i].port = port;
            return;
        }
    }
}