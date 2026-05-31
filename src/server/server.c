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



int main(){
    printf("===========================================\n");
    printf("                 LAVAGNA                   \n");
    printf("===========================================\n\n");

    

    if (server_setup() != 0){
        printf("[ERROR]: errore nella configurazione del server\n");
        exit(-1);
    }

    initialize_lavagna();

    printf("[SERVER]: Configurazione andata a buon fine.\n");
    
    printf("[SERVER]: server pronto\n");

    time_t last_ping_check = time(NULL);
    Message msg;
    int sender_fd;

    while(1){
        check_net();


        while((sender_fd = get_message_from_net(&msg)) != -1){
            if(sender_fd != STDIN_FILENO){
                printf("[%d]: %s\n",msg.sender_port, command_to_string(msg.type));
            }
            handle_command(&msg, sender_fd);
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