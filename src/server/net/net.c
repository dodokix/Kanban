#include "net.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>
#include "../../shared/constants/net_constants.h"
#include "../../shared/constants/core_constants.h"
#include "../../shared/cmd/cmd.h"

#define STDIN 0

int sockfd;

typedef struct {
    int socket;
    uint16_t port;
    char buff[CMD_BUFF_SIZE];
    int n_byte;
} client;

client clients_arr[MAX_CLIENTS] = {0};

client cmd_interface = {
    .socket = STDIN_FILENO,
    .port = 0,
    .n_byte = 0
};


fd_set masterfds;
fd_set readfds;
int maxfd;


int configure_port(){
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
        perror("Socket failed!");
        return -1;
    }

    memset(&address, 0, addrlen);
    address.sin_family = AF_INET;
    address.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_ADDRESS, &address.sin_addr);

    if((bind(sockfd, (struct sockaddr*)&address, addrlen)) < 0 ){
        perror("Bind failed!");
        close(sockfd);
        return -1;
    }

    
    if((listen(sockfd, BACKLOG)) < 0){
        perror("Listen failed!");
        close(sockfd);
        return -1;
    }

    printf("Server listening on port %d...\n", SERVER_PORT);
    return 0;
}

int server_setup(){

    if(configure_port() != 0){
        printf("socket initialization failed\n");
        return -1;
    }

    FD_ZERO(&masterfds);

    FD_SET(sockfd, &masterfds);
    FD_SET(STDIN_FILENO, &masterfds);
    maxfd = sockfd;
    FD_ZERO(&readfds);
    return 0;
}

void add_client(int socket, uint16_t port){
     for(int i = 0; i < MAX_CLIENTS; ++i){
        if(clients_arr[i].socket == 0){
            clients_arr[i].socket = socket;
            clients_arr[i].port = port;
            printf("aggiungo nuovo client alla lista,\n");
            FD_SET(socket, &masterfds);
            maxfd = (socket > maxfd) ? socket : maxfd;
            break;
        }
    }
}

void remove_client(client* client){
    int socket = client->socket;

    client->socket = 0;
    client->port = 0;

    FD_CLR(socket, &masterfds);
    close(socket);
    int i;
    for(i = FD_SETSIZE - 1; i >= 0 && !FD_ISSET(i, &masterfds); ++i);
    maxfd = i;
}

void accept_net(){
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int new_sock;

    if((new_sock = accept(sockfd, (struct sockaddr*)&client_addr, (socklen_t*)&client_len)) < 0){
        perror("Accept failed!\n");
        exit(EXIT_FAILURE);
    }
    uint16_t porta_client = ntohs(client_addr.sin_port);
    printf("Nuova connessione stabilita con client! porta: %d \n", porta_client);
    add_client(new_sock, porta_client);
}

void new_connection(){
    struct sockaddr_in client_addr;
    socklen_t client_lenght = sizeof(client_addr);

    int client_socket = accept(sockfd, (struct sockaddr*)&client_addr, &client_lenght);
    if(client_socket < 0){
        printf("errore nell'accept!\n");
        return;
    }
    add_client(client_socket, ntohs(client_addr.sin_port));
}

void cmd_stdin(){
    int space_left = CMD_BUFF_SIZE - 1 - cmd_interface.n_byte;
    if(space_left == 0){
        printf("buffer stdin pieno\n");
        return;
    }
    char* buff_pt = cmd_interface.buff + cmd_interface.n_byte;
    int n = read(cmd_interface.socket, buff_pt, space_left);

    if(n <= 0){
        return n;
    }

    cmd_interface.n_byte += n;

    char* buffer = cmd_interface.buff;
    int n_bytes = cmd_interface.n_byte;

    buffer[strcspn(buffer, '\n')] = "\0";
    if(strlen(buffer) == 0)
        return;

    handle_command(cmd_interface.socket, buffer);
}

void cmd_client(int fd){
    
}


int check_net(){
    
    readfds = masterfds;

    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    int activity = select(maxfd + 1, &readfds, NULL, NULL, &tv);
    if(activity < 0){
        perror("Errore nella select\n");
        return 0;
    }
    else if(activity == 0){
        printf("Timeout scaduto, nessun messaggio ricevuto entro il tempo limite\n");
        // gestisci timeout
    }
    else{
        for(int i = 0; i <= maxfd; ++i){
            if(FD_ISSET(i, &readfds)){
                if(i == STDIN_FILENO){
                    cmd_stdin();
                }
                else if(i == sockfd){
                    new_connection();
                }
                else{
                    //cmd_client();
                }
            }
        }
    }
}



void close_net(){
    
}