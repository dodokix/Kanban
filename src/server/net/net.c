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
#include "../../shared/constants/server_constants.h"

int sockfd;
int maxfd;
int client_socket[MAX_CLIENTS] = {0};
fd_set readfds;


int configure_port(){
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
        perror("Socket failed!");
        return -1;
    }

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = INADDR_ANY;

    if((bind(sockfd, (struct sockaddr*)&address, addrlen)) < 0 ){
        perror("Bind failed!");
        close(sockfd);
        return -1;
    }

    
    if((listen(sv_fd, BACKLOG)) < 0){
        perror("Listen failed!");
        close(sockfd);
        return -1;
    }

    printf("Server listening on port %d...\n", PORT);
    return 0;
}

int check_net(){
    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);
    int activity;
    maxfd = sockfd;

    for(int i = 0; i<MAX_CLIENTS; ++i){
        sd = client_socket[i];
        if(sd > 0){
            FD_SET(sd, &readfds);
        }
        if(sd > max_sd){
            max_sd = sd;
        }
    }

    activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
    if((activity < 0) && (errno != EINTR)) {
        perror("select failed!\n");
    }

    if(FD_ISSET(sockfd, &readfds)){
        accept_net();
    }
}

void accept_net(){
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int new_sock;

    if((new_sock = accept(sv_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0){
        perror("Accept failed!\n");
        exit(EXIT_FAILURE);
    }
    int porta_client = ntohs(client_addr.sin_port);

    printf("Nuova connessione stabilita con client! porta: %d \n", porta_client);

    for(int i = 0; i < MAX_CLIENTS; ++i){
        if(client_socket[i] == 0){
            client_socket[i] = new_sock;
            printf("aggiungo nuovo client alla lista,\n");
            break;

        }
    }

}

void close_net(){
    
}