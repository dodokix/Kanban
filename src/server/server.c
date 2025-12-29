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
#include "../../shared/lavagna.h"



int main(int argc, char*argv[]){
    Lavagna *lav = initialize_lav();
    int max_sd, sd, activity;
    int new_sock, *socket_thread;
    char buffer[BUFFER_SIZE];

    configure_port();

    while(1){
        
        check_net();



        for (int i = 0; i < MAX_CLIENTS; i++) {
            sd = client_socket[i];
            if (FD_ISSET(sd, &readfds)) {
                int valread = read(sd, buffer, BUFFER_SIZE - 1);
                if (valread <= 0) {
                    // Il client ha chiuso la connessione
                    printf("Client disconnesso (socket fd: %d)\n", sd);
                    //GESTIONE CARD IN DOING.
                    close(sd);
                    client_socket[i] = 0;
                } else {
                    buffer[valread] = '\0';
                    char *cmd = strtok(buffer, " ");
                    char *args = strtok(NULL, "");

                    if(cmd != NULL && strcmp(cmd, "HELLO") == 0){
                        cmd_hello(sd, args, lav);
                    }
                    else if(cmd != NULL && strcmp(cmd, "QUIT") == 0){
                        cmd_quit(sd, lav);
                        FD_CLR(sd, &readfds);
                        close(sd);
                        client_socket[i] = 0;
                        printf("connessione chiusa correttamente dopo QUIT.\n");
                    }

                }
            }
        }
    }

    return 0;
}