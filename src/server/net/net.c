#include "net.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>
#include <stdbool.h>
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

// typedef struct {
//     int client_fd;
//     uint16_t port;
//     char buffer[CMD_BUFF_SIZE];
// } ServerEvent;


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
        exit(-1);
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
    for(i = maxfd; i >= 0 && !FD_ISSET(i, &masterfds); --i);
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

//
int read_stdin(){

    int space_left = CMD_BUFF_SIZE - 1 - cmd_interface.n_byte;

    if(space_left == 0){
        printf("buffer stdin pieno\n");
        return 0;
    }
    char* buff_pt = cmd_interface.buff + cmd_interface.n_byte;
    int n = read(cmd_interface.socket, buff_pt, space_left);

    if(n <= 0){
        return n;
    }

    cmd_interface.n_byte += n;
    return 0;
}

client* find_client_by_socket(int fd){
    for(int i = 0; i < MAX_CLIENTS; ++i){
        if(clients_arr[i].socket == fd){
            return &clients_arr[i];
        }
    }
    printf("client non torvato nella funzione  find_client_by_socket\n");
    return NULL;
}

int read_client(int fd){
    client* sender = find_client_by_socket(fd);

    int space_left = CMD_BUFF_SIZE - 1 - sender->n_byte;

    if(space_left == 0){
        printf("buffer dei comandi pieno\n");
        return 0;
    }
    char* buff_pt = sender->buff + sender->n_byte;
    int n = recv(fd, buff_pt, space_left, 0);

    if(n < 0){
        return n;
    }
    else if(n == 0){
        printf("porta %d si e' disconnesso\n", sender->port);
        FD_CLR(fd, &masterfds);
        close(fd);
        if(fd == maxfd){
            while(!FD_ISSET(maxfd, &masterfds) && maxfd > 0)
                maxfd--;
        }
        sender->socket = 0;
        sender->n_byte = 0;
        sender->port = 0;
        memset(sender->buff, 0, CMD_BUFF_SIZE);
        return 1;
    }
    return 0;
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
    
    if(FD_ISSET(STDIN_FILENO, &readfds)){
            read_stdin();
    }
        
    if(FD_ISSET(sockfd, &readfds)){
            new_connection();
        }
        
    for(int i = 0; i <= MAX_CLIENTS; ++i){
        int socket = clients_arr[i].socket;
        if(socket > 0 && FD_ISSET(socket, &readfds)){
            read_client(socket);
        }
    }

    return 0;
}

void remove_cmd_from_buffer(client* c, int cmd_length){
    int remaining_bytes = c->n_byte - (cmd_length + 1);
        if(remaining_bytes >= 0){
        memmove(c->buff, c->buff + cmd_length + 1, remaining_bytes);
    }
    c->n_byte = remaining_bytes;
    memset(c->buff + c->n_byte, 0, CMD_BUFF_SIZE - c->n_byte);

}

int extract_line_from_buffer(client* c, char* dest_buffer, int max_length){
    if(c->n_byte == 0) return 0;

    int cmd_len;
    bool found = false;
    for(int i = 0; i < c->n_byte; ++i){
        if(c->buff[i] == '\n'){
            cmd_len = i;
            found = true;
            c->buff[i] = '\0';
            break;
        }
    }

    if(found){
        if(cmd_len > max_length || cmd_len == 0){
            printf("comando non valido troppo lungo o vuoto \n");
            remove_cmd_from_buffer(c, cmd_len);
            return -1;
        }
        
        memcpy(dest_buffer, c->buff, cmd_len);
        dest_buffer[cmd_len] = '\0';
        remove_cmd_from_buffer(c, cmd_len);
        return 1;
    }

    return 0;
}

int get_command_from_net(ServerEvent* event){

    if(extract_line_from_buffer(&cmd_interface, event->buffer, CMD_BUFF_SIZE) > 0){
        event->client_fd = STDIN_FILENO;
        event->port = 0;
        return 1;
    }

    for(int i = 0; i < MAX_CLIENTS; ++i){
        if(clients_arr[i].socket > 0){
            if(extract_line_from_buffer(&clients_arr[i], event->buffer, CMD_BUFF_SIZE) > 0){
                event->client_fd = clients_arr[i].socket;
                event->port = clients_arr[i].port;
                return 1;
            }
        }
    }

    return 0;
}

void close_net(){
    if(sockfd > 0){
        close(sockfd);
        sockfd = 0;
    }

    for(int i = 0; i < MAX_CLIENTS; ++i){
        if(clients_arr[i].socket != 0){
            close(clients_arr[i].socket);
        }
    }

    printf("chiusura socket completata!\n");
}

