#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <stdio.h>      // Per printf
#include <string.h>     // Per memset
#include <arpa/inet.h>  // Per inet_pton, AF_INET
#include <unistd.h>     // Per close
#include "net.h"

#define BUFFER_SIZE 1024
#define SERVER_PORT 5678

int server_fd;
int listen_fd;
int port;
struct sockaddr_in usr_addr;

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
        printf("Indirizzo non valido\n");
        return -1;
    }

    if(connect(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        printf("Connessione fallita\n");
        return -1;
    }

    send(server_fd, "HELLO", sizeof("HELLO"), 0);
    close(server_fd);
    return 0;
}