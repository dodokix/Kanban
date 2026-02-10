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

    initialize_lavagna();

    if (server_setup() != 0){
        printf("[ERROR]: errore nella configurazione del server\n");
        exit(-1);
    }

    printf("[SERVER]: Configurazione andata a buon fine.\n");

    time_t last_ping_check = time(NULL);

    while(1){
        check_net();

        Message msg;
        while(get_message_from_net(&msg)){
            printf("[RICEVUTO da FD %d]: %s\n",event.port, event.buffer);
            handle_command(&msg);
        }

        time_t now = time(NULL);
        if(difftime(now, last_ping_check) > 10){
            check_ping_timeouts();
            last_ping_check = now;
        }
    }

    // gestione periodica per PING
    close_net();
    return 0;
}