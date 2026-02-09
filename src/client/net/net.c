#include "net.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#define BUFFER_SIZE 1024
#define SERVER_PORT 5678
#define BACKLOG 10

struct sockaddr_in usr_addr;

int server_fd = -1;
int listen_fd = -1;
int my_port = 0;
PeerConnection peers[MAX_USERS];
int num_peers = 0;

fd_set masterfds;
fd_set readfds;
int maxfd;

int setup_server_socket(){
    struct sockaddr_in server_addr;

    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("Errore nella creazione del socket\n");
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);    

    if(inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0){
        printf("[ERROR]:Indirizzo server non valido\n");
        return -1;
    }

    if(connect(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        printf("[ERROR]:Connessione fallita\n");
        return -1;
    }
    return 0;
}

int setup_p2p_socket(int myport){
    struct sockaddr_in client_addr;

    if((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("[ERROR]: errore nella creazione del socket di ascolto\n");
        return -1;
    }

    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr = INADDR_ANY;
    client_addr.sin_port = htons(myport);

    if(bind(listen_fd, (struct sockaddr*)&client_addr, sizeof(client_addr)) < 0){
        printf("[ERROR]: errore nella bind\n");
        return -1;
    }

    if(listen(listen_fd, BACKLOG) < 0){
        printf("[ERROR]: errore nella listen\n");
        return -1;
    }
    printf("[NET] In ascolto sulla porta %d per connessioni P2P\n", myport);
    return 0;
}

int client_setup(int port){
    my_port = port;
    num_peers = 0;

    for(int i = 0; i < MAX_CLIENTS; ++i){
        peers[i].port = 0;
        peers[i].socket = -1;
    }
    
    if(setup_server_socket() != 0){
        perror("[ERROR]: connessione al server fallita\n");
        exit(-1);
    }

    if(setup_p2p_socket(myport) != 0){
        perror("[ERROR]: fallita inizializzazione del socket di ascolto\n");
        exit(-1);
    }

    FD_ZERO(&masterfds);
    FD_SET(STDIN_FILENO, &masterfds);
    FD_SET(server_fd, &masterfds);   
    FD_SET(listen_fd, &masterfds);
    maxfd = (server_fd > listen_fd) ? server_fd : listen_fd;

    return 0;
}

int send_to_server(const Message* msg) {
    char buffer[BUFFER_SIZE];
    int len;
    
    // Serializza messaggio
    len = serialize_message(msg, buffer, BUFFER_SIZE);
    if(len < 0) {
        fprintf(stderr, "[ERROR] Errore nella serializzazione del messaggio\n");
        return -1;
    }
    
    // Invia
    if(send(server_fd, buffer, len, 0) < 0) {
        perror("send");
        return -1;
    }
    
    printf("[NET] Inviato a lavagna: %s", buffer);
    return 0;
}

int send_to_peer(int peer_port, const Message* msg) {
    char buffer[BUFFER_SIZE];
    int len;
    int peer_socket;
    
    // Trova socket del peer
    peer_socket = get_peer_socket(peer_port);
    
    // Se non è connesso, prova a connettersi
    if(peer_socket < 0) {
        peer_socket = connect_to_peer(peer_port);
        if(peer_socket < 0) {
            fprintf(stderr, "[ERROR] Impossibile connettersi al peer %d\n", peer_port);
            return -1;
        }
    }
    
    // Serializza messaggio
    len = serialize_message(msg, buffer, BUFFER_SIZE);
    if(len < 0) {
        fprintf(stderr, "[ERROR] Errore nella serializzazione del messaggio\n");
        return -1;
    }
    
    // Invia
    if(send(peer_socket, buffer, len, 0) < 0) {
        perror("send");
        return -1;
    }
    
    printf("[NET] Inviato a peer %d: %s", peer_port, buffer);
    return 0;
}

int broadcast_to_peers(const Message* msg, const int* peer_list, int num_peers_list) {
    for(int i = 0; i < num_peers_list; i++) {
        if(peer_list[i] != my_port) {  // Non invia a se stesso
            send_to_peer(peer_list[i], msg);
        }
    }
    return 0;
}

int connect_to_peer(int peer_port) {
    struct sockaddr_in peer_addr;
    int sock;
    
    // Verifica se già connesso
    if(get_peer_socket(peer_port) >= 0) {
        return get_peer_socket(peer_port);
    }
    
    // Crea socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0) {
        perror("socket");
        return -1;
    }
    
    // Configura indirizzo
    memset(&peer_addr, 0, sizeof(peer_addr));
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_port = htons(peer_port);
    
    if(inet_pton(AF_INET, "127.0.0.1", &peer_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sock);
        return -1;
    }
    
    // Connessione
    if(connect(sock, (struct sockaddr*)&peer_addr, sizeof(peer_addr)) < 0) {
        perror("connect");
        close(sock);
        return -1;
    }
    
    // Salva connessione
    if(num_peers < MAX_USERS) {
        peers[num_peers].port = peer_port;
        peers[num_peers].socket = sock;
        num_peers++;
        printf("[NET] Connesso al peer %d\n", peer_port);
    }
    
    return sock;
}

int receive_message(int socket_fd, Message* msg) {
    char buffer[BUFFER_SIZE];
    int bytes_read;
    
    // Legge dal socket
    bytes_read = recv(socket_fd, buffer, BUFFER_SIZE - 1, 0);
    if(bytes_read < 0) {
        perror("recv");
        return -1;
    }
    
    if(bytes_read == 0) {
        // Connessione chiusa
        return 0;
    }
    
    buffer[bytes_read] = '\0';
    
    // Deserializza
    if(deserialize_message(buffer, msg) < 0) {
        fprintf(stderr, "[ERROR] Errore nella deserializzazione del messaggio\n");
        return -1;
    }
    
    return bytes_read;
}

int accept_peer_connection() {
    struct sockaddr_in peer_addr;
    socklen_t addr_len = sizeof(peer_addr);
    int new_socket;
    
    new_socket = accept(listen_fd, (struct sockaddr*)&peer_addr, &addr_len);
    if(new_socket < 0) {
        perror("accept");
        return -1;
    }
    
    int peer_port = ntohs(peer_addr.sin_port);
    
    // Salva connessione se c'è spazio
    if(num_peers < MAX_USERS) {
        peers[num_peers].port = peer_port;
        peers[num_peers].socket = new_socket;
        num_peers++;
        printf("[NET] Nuova connessione P2P accettata (porta remota: %d)\n", peer_port);
    } else {
        fprintf(stderr, "[WARN] Troppi peer connessi, connessione rifiutata\n");
        close(new_socket);
        return -1;
    }
    
    return new_socket;
}

void close_peer_connection(int peer_port) {
    for(int i = 0; i < num_peers; i++) {
        if(peers[i].port == peer_port) {
            close(peers[i].socket);
            // Sposta gli elementi successivi
            for(int j = i; j < num_peers - 1; j++) {
                peers[j] = peers[j + 1];
            }
            num_peers--;
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

int get_peer_socket(int peer_port) {
    for(int i = 0; i < num_peers; i++) {
        if(peers[i].port == peer_port) {
            return peers[i].socket;
        }
    }
    return -1;
}

