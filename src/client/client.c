#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "net/net.h"

#define BUFFER_SIZE 1024


int main(int argc, char *argv[]){
    if(argc < 2){
        printf("uso: %s <porta>\n", argv[0]);
        exit(-1);
    }
    int my_port = atoi(argv[1]);

    printf("Kanban avviato sulla porta: %d\n", my_port);

    if(client_setup(my_port) != 0){
        printf("[ERROR]: impossibile avviare il client\n");
        exit(-1);
    }

    send_hello();
    
    printf("\nComandi disponibili:\n");
    print_help();
    
    // Prepara set di file descriptor per select
    fd_set master_fds, read_fds;
    int max_fd;
    
    FD_ZERO(&master_fds);
    FD_SET(STDIN_FILENO, &master_fds);  // stdin
    FD_SET(server_fd, &master_fds);      // socket lavagna
    FD_SET(listen_fd, &master_fds);      // socket ascolto P2P
    
    max_fd = (server_fd > listen_fd) ? server_fd : listen_fd;
    

    while(1) {
        read_fds = master_fds;
        
        // Aggiungi socket dei peer
        for(int i = 0; i < num_peers; i++) {
            FD_SET(peers[i].socket, &read_fds);
            if(peers[i].socket > max_fd) {
                max_fd = peers[i].socket;
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
        
        if(activity == 0) {
            // Timeout - nessuna attività
            continue;
        }
        
        // Controlla stdin
        if(FD_ISSET(STDIN_FILENO, &read_fds)) {
            handle_stdin();
        }
        
        // Controlla messaggi dalla lavagna
        if(FD_ISSET(server_fd, &read_fds)) {
            Message msg;
            int ret = receive_message(server_fd, &msg);
            if(ret > 0) {
                handle_server_message(&msg);
            } else if(ret == 0) {
                printf("[INFO] Lavagna disconnessa\n");
                break;
            }
        }
        
        // Controlla nuove connessioni P2P
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
            if(FD_ISSET(peers[i].socket, &read_fds)) {
                Message msg;
                int ret = receive_message(peers[i].socket, &msg);
                if(ret > 0) {
                    handle_peer_message(&msg, peers[i].socket);
                } else if(ret == 0) {
                    printf("[INFO] Peer %d disconnesso\n", peers[i].port);
                    FD_CLR(peers[i].socket, &master_fds);
                    close_peer_connection(peers[i].port);
                    i--; // Decrementa perché l'array è stato compattato
                }
            }
        }
    }
    
    // Cleanup
    net_cleanup();
    return 0;
}