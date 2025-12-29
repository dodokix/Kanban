#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUFFER_SIZE 1024

int configure_socket();


int main(int argc, char *argv[]){
    int sock;
    int port = atoi(argv[1]);
    struct sockaddr_in usr_addr, sv_addr;
    char buffer[BUFFER_SIZE] = {0};

    if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("Errore nella creazione del socket\n");
        return -1;
    }

    memset(&usr_addr, 0, sizeof(usr_addr));
    usr_addr.sin_family = AF_INET;
    usr_addr.sin_port = htons(port);    

    if(inet_pton(AF_INET, "127.0.0.1", &usr_addr.sin_addr) <= 0){
        printf("Indirizzo non valido\n");
        return -1;
    }

    if((bind(sock, (struct sockaddr *)&usr_addr, sizeof(usr_addr))) < 0 ){
        printf("errore nella bind");
        return -1;
    }

    if(connect(sock, (struct sockaddr *)&usr_addr, sizeof(usr_addr)) < 0){
        printf("Connessione fallita\n");
        return -1;
    }

    while(1){
        char *hello = "HELLO";
        send(sock, hello, strlen(hello), 0);

        int valread = read(sock, buffer, BUFFER_SIZE);
        buffer[valread] = '\0';
        printf("risposta dal server: %s\n", buffer);
        break;
    }

    return 0;
}
