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



int main(int argc, char*argv[]){
    printf("inizio configurazionen\n");
    ServerEvent event;

    if (server_setup() != 0){
        printf("[ERROR]: errore nella configurazione del server\n");
        exit(-1);
    }

    printf("Configurazione andata a buon fine.\n");

    while(1){
        check_net();

        while(get_command_form_net(&event)){
            printf("[RICEVUTO da FD %d]: %s\n",event.socket, event.buffer);
        }
    }

    // gestione periodica per PING


}