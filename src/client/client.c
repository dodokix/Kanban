#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include "net/net.h"
#include "core/core.h"

#define BUFFER_SIZE 1024


int main(int argc, char *argv[]){
    srand(time(NULL));

    if(argc < 2){
        printf("uso: %s <porta>\n", argv[0]);
        exit(-1);
    }

    int my_port = atoi(argv[1]);

    printf("===========================================\n");
    printf("         KANBAN UTENTE (porta %d)          \n", my_port);
    printf("===========================================\n\n");

    if(client_setup(my_port) != 0){
        printf("[ERROR]: impossibile avviare il client\n");
        exit(-1);
    }

    send_hello();
    print_help();
    
    fd_set master_fds, read_fds;    
    FD_ZERO(&master_fds);
    FD_SET(STDIN_FILENO, &master_fds);  
    FD_SET(server_fd, &master_fds);     
    FD_SET(listen_fd, &master_fds); 
    
    int max_fd = (server_fd > listen_fd) ? server_fd : listen_fd;
    
    while(1) {
        read_fds = master_fds;
        
        // Aggiungi socket dei peer
        for(int i = 0; i < num_peers; i++) {
            if(peers[i].socket >= 0){
                FD_SET(peers[i].socket, &read_fds);
                if(peers[i].socket > max_fd) {
                    max_fd = peers[i].socket;
                }
            }
        }
        
        // Select con timeout
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        
        int activity = select(max_fd + 1, &read_fds, NULL, NULL, &tv);
        
        if(activity < 0) {
            perror("select");
            break;
        }
        
        //timeout
        if(activity == 0) continue;
        
        // comando da tastiera
        if(FD_ISSET(STDIN_FILENO, &read_fds)) {
            handle_stdin();
        }
        
        // messaggio dal server
        if(FD_ISSET(server_fd, &read_fds)) {
            Message msg;
            int ret = receive_message(&msg, server_fd);
            if(ret > 0) {
                handle_server_message(&msg);
            } else if(ret == 0) {
                printf("[INFO] Lavagna disconnessa\n");
                break;
            }
        }
        
        //nuova connessione p2p in ingresso
        if(FD_ISSET(listen_fd, &read_fds)) {
            int new_socket = accept_peer_connection();
            if(new_socket >= 0) {
                FD_SET(new_socket, &master_fds);
                if(new_socket > max_fd) {
                    max_fd = new_socket;
                }
            }
        }
        
        // Controlla messaggi dai peer
        for(int i = 0; i < num_peers; i++) {
            int peer_socket = peers[i].socket;
            if(peer_socket < 0)continue;

            if(FD_ISSET(peer_socket, &read_fds)) {
                Message msg;
                int ret = receive_message(&msg, peer_socket);
                if(ret > 0) {
                    handle_peer_message(&msg, peer_socket);
                } else if(ret == 0) {
                    printf("[INFO] Peer %d disconnesso\n", peers[i].port);
                    FD_CLR(peer_socket, &master_fds);
                    close_peer_connection(peers[i].port);
                    i--; // decremento perche' l'array e' stato compattato
                }
            }
        }
    }
    
    net_cleanup();
    return 0;
}