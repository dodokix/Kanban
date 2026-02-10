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
#include <stdint.h>
#include "net/net.h"
#include "core/core.h"



int main(int argc, char*argv[]){
    printf("===========================================\n");
    printf("                 LAVAGNA                   \n");
    printf("===========================================\n\n");

    

    if (server_setup() != 0){
        printf("[ERROR]: errore nella configurazione del server\n");
        exit(-1);
    }

    printf("Configurazione andata a buon fine.\n");

    while(1){
        check_net();

        while(get_command_from_net(&event)){
            printf("[RICEVUTO da FD %d]: %s\n",event.port, event.buffer);
            handle_command(&event);
        }
    }

    // gestione periodica per PING
    close_net();
    return 0;
}