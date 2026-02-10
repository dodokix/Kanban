#include "core.h"
#include "net.h"
#include "protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "../net/net.h"

// Stato dell'asta corrente
typedef struct {
    int card_id;
    int my_cost;
    int received_count;
    int expected_count;
    struct {
        int port;
        int cost;
    } bids[MAX_USERS];
    bool active;
} AuctionState;

AuctionState current_auction = {0};

void handle_server_message(Message* msg) {
    switch(msg->type) {
        case CMD_AVAILABLE_CARD: {
            printf("\n[ASTA] Nuova card disponibile: ID %d - '%s'\n", msg->card_id, msg->text);
            
            // 1. Setup Asta
            current_auction.active = true;
            current_auction.card_id = msg->card_id;
            current_auction.expected_count = msg->num_users; // Numero peer
            current_auction.received_count = 0;
            
            // 2. Genera Costo Casuale
            srand(time(NULL) ^ my_port);
            current_auction.my_cost = rand() % 100;
            printf("[ASTA] Il mio costo: %d. Peer da contattare: %d\n", current_auction.my_cost, msg->num_users);

            // 3. Invia CHOOSE_USER a tutti i peer
            Message bid_msg = {0};
            bid_msg.type = CMD_CHOOSE_USER;
            bid_msg.sender_port = my_port;
            bid_msg.cost = current_auction.my_cost;
            bid_msg.card_id = msg->card_id;

            for(int i=0; i<msg->num_users; i++) {
                int peer = msg->user_list[i];
                send_to_peer(peer, &bid_msg);
            }
            
            // Se non ci sono altri peer valuta subito
            if(msg->num_users == 0) check_auction_result();
            break;
        }
        
        case CMD_ACK_HELLO:
            printf("[INFO] Registrato correttamente alla lavagna.\n");
            break;
            
        default:
            printf("[INFO] Messaggio dalla lavagna: %s\n", command_to_string(msg->type));
    }
}

void handle_peer_message(Message* msg, int peer_socket) {
    if(msg->type == CMD_CHOOSE_USER) {
        if(!current_auction.active || msg->card_id != current_auction.card_id) return;
        
        printf("[ASTA] Ricevuto costo %d dal peer %d\n", msg->cost, msg->sender_port);
        
        // Salva l'offerta
        int idx = current_auction.received_count++;
        current_auction.bids[idx].port = msg->sender_port;
        current_auction.bids[idx].cost = msg->cost;
        
        // Controlla se abbiamo finito
        if(current_auction.received_count >= current_auction.expected_count) {
            check_auction_result();
        }
    }
}

void check_auction_result() {
    int min_cost = current_auction.my_cost;
    int winner = my_port;
    
    // Trova il vincitore
    for(int i=0; i<current_auction.received_count; i++) {
        int other_cost = current_auction.bids[i].cost;
        int other_port = current_auction.bids[i].port;
        
        if(other_cost < min_cost) {
            min_cost = other_cost;
            winner = other_port;
        } else if (other_cost == min_cost) {
            // spareggio su porta minore
            if(other_port < winner) {
                winner = other_port;
            }
        }
    }
    
    current_auction.active = false;
    
    if(winner == my_port) {
        printf("\n[WIN] Ho vinto l'asta per la card %d! (Costo: %d)\n", current_auction.card_id, min_cost);
        
        //Notifica Presa in carico
        Message ack = {0};
        ack.type = CMD_ACK_CARD;
        ack.sender_port = my_port;
        ack.card_id = current_auction.card_id;
        send_to_server(&ack);
        
        //Esegui lavoro
        printf("[WORK] Esecuzione task in corso...\n");
        sleep(5); // Simulazione lavoro
        
        //Notifica Completamento
        Message done = {0};
        done.type = CMD_CARD_DONE;
        done.sender_port = my_port;
        done.card_id = current_auction.card_id;
        send_to_server(&done);
        printf("[WORK] Task completato e notificato.\n");
        
    } else {
        printf("\n[LOSE] Asta vinta da %d con costo %d.\n", winner, min_cost);
    }
}

void handle_stdin() {
    char buffer[256];
    if(read(STDIN_FILENO, buffer, sizeof(buffer)) > 0) {
        char* tok = strtok(buffer, " ");

        if(strncmp(tok, "quit", strlen(tok)) == 0) exit(0);
        if(strncmp(tok, "create", strlen(tok)) == 0){
            tok = strtok(buffer, "\n");
            Message msg;
            memset(&msg, 0, sizeof(msg));
            msg.type = CMD_CREATE_CARD;
            strncpy(msg.text, tok);
            msg.sender_port = my_port;
            send_to_server(&msg);
            printf("[CREATE]: inviata card ta creare\n");
        } 
        if(strncmp(tok, "help", strlen(tok)) == 0) print_help();
    }
}

void send_hello() {
    Message msg;
    memset(&msg, 0, sizeof(Message));
    
    msg.type = CMD_HELLO;
    msg.sender_port = my_port;
    
    send_to_server(&msg);
    printf("[CMD] HELLO inviato alla lavagna\n");
}

void print_help() {
    printf("  quit              - Esci dal programma\n");
    printf("  create <testo>    - Crea una nuova card\n");
    printf("  status            - Mostra stato corrente\n");
    printf("  help              - Mostra questo messaggio\n");
    printf("\n");
}