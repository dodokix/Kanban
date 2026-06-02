#include "core.h"
#include "../net/net.h"
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "../../shared/protocol/protocol.h"

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
    int user_list[MAX_USERS];
    bool active;
} AuctionState;

AuctionState current_auction = {0};

void handle_server_message(Message* msg) {
    switch(msg->type) {
        case CMD_AVAILABLE_CARD: {
            
            printf("\n[ASTA] Nuova card disponibile: ID %d - '%s'\n", msg->card_id, msg->text);
            current_auction.active = false;
            current_auction.card_id = msg->card_id;
            current_auction.expected_count = msg->num_users;
            current_auction.received_count = 0;

            for(int i = 0; i<msg->num_users; ++i){
                current_auction.user_list[i] = msg->user_list[i];
            }

            Message ack_msg;
            memset(&ack_msg, 0, sizeof(Message));
            ack_msg.type = CMD_READY;
            ack_msg.sender_port = my_port;
            ack_msg.card_id = msg->card_id;
            send_to_server(&ack_msg);
            break;
        }

        case CMD_START_AUCTION: {
            //if(current_auction.card_id != msg->card_id) break;

            current_auction.active = true;
            current_auction.my_cost = rand();
            printf("[ASTA] Il mio costo: %d.\n", current_auction.my_cost);

            Message bid_msg;
            memset(&bid_msg, 0, sizeof(Message));
            bid_msg.type = CMD_CHOOSE_USER;
            bid_msg.sender_port = my_port;
            bid_msg.cost = current_auction.my_cost;
            bid_msg.card_id = msg->card_id;

            for(int i=0; i < current_auction.expected_count; i++) {
                if(current_auction.user_list[i] <= 0){
                    printf("[DEBUG] ignorato peer con portta non valida: %d\n",msg->user_list[i]);
                    continue;
                }
                send_to_peer(&bid_msg, msg->user_list[i]);
            }            
            break;
        }
        
        case CMD_HELLO:
            printf("[INFO] Registrato correttamente alla lavagna.\n");
            break;

        case CMD_PING_USER: {
            printf("[PING] Ricevuto ping dalla lavagna per card %d\n", msg->card_id);
            Message pong;
            memset(&pong, 0, sizeof(Message));
            pong.type = CMD_PONG_LAVAGNA;
            pong.sender_port = my_port;
            pong.card_id = msg->card_id;
            send_to_server(&pong);
            printf("[PONG] inviato pong alla lavagnan\n");
            break;
        }

        case CMD_QUIT: {
                printf("[QUIT] Il server ha rifiutato la connessione: %s \n", msg->text);
                break;
        }
            
        default:
            break;
    }
}

void handle_peer_message(Message* msg, int peer_sock) {
    if(msg->type != CMD_CHOOSE_USER) return;
    
    update_peer_port(peer_sock, msg->sender_port);

    if(!current_auction.active || msg->card_id != current_auction.card_id)
        return;

    if(current_auction.received_count >= MAX_USERS)
        return;
    
    printf("[ASTA] Ricevuto costo %d dal peer %d\n", msg->cost, msg->sender_port);
    
    int i = current_auction.received_count++;
    current_auction.bids[i].port = msg->sender_port;
    current_auction.bids[i].cost = msg->cost;
    
    if(current_auction.received_count >= current_auction.expected_count) {
        check_auction_result();
    }
}

void check_auction_result() {
    int min_cost = current_auction.my_cost;
    int winner = my_port;
    
    for(int i=0; i < current_auction.received_count; i++) {
        int next_cost = current_auction.bids[i].cost;
        int next_port = current_auction.bids[i].port;
        
        if(next_cost < min_cost) {
            min_cost = next_cost;
            winner = next_port;
        } else if (next_cost == min_cost && next_port < winner) {
                winner = next_port;
        }
    }
    
    current_auction.active = false;
    
    if(winner == my_port) {
        printf("\n[WIN] Ho vinto l'asta per la card %d! (Costo: %d)\n", current_auction.card_id, min_cost);
        
        Message ack = {0};
        ack.type = CMD_ACK_CARD;
        ack.sender_port = my_port;
        ack.card_id = current_auction.card_id;
        send_to_server(&ack);
        
        printf("[WORK] Esecuzione task in corso...\n");
        sleep(5);
        
        Message done;
        memset(&done, 0, sizeof(Message));
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
    if(read(STDIN_FILENO, buffer, sizeof(buffer) - 1) <= 0) return;

    buffer[strcspn(buffer, "\n")] = '\0';
    int buffer_len = strlen(buffer);

    if(strncmp(buffer, "quit", buffer_len) == 0){
        Message quit_msg;
        memset(&quit_msg, 0, sizeof(Message));
        quit_msg.type = CMD_QUIT;
        quit_msg.sender_port = my_port;
        send_to_server(&quit_msg);
        printf("[INFO] Disconnessione...\n");
        sleep(1);
        net_cleanup();
        exit(0);
    }
    if(strncmp(buffer, "create", strlen("create")) == 0){
        const char* card_text = buffer + strlen("create");
        if(strlen(card_text) == 0){
            printf("[ERROR] Sintassi: create <testo attivita'>\n");
            return;
        }
        Message msg;
        memset(&msg, 0, sizeof(Message));
        msg.type = CMD_CREATE_CARD;
        msg.sender_port = my_port;
        strncpy(msg.text, card_text, MAX_TEXT_LEN - 1);
        send_to_server(&msg);
        printf("[CREATE]: inviata card ta creare\n");
        return;
    } 

    if(strncmp(buffer, "help", buffer_len) == 0){ 
        print_help();
        return;
    }

    if(strlen(buffer) > 0){
        printf("[WARN] Comando non riconosciuto: '%s'\n", buffer);
        print_help();
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
    printf("\nComandi disponibili:\n");
    printf("\n  quit              - Esci dal programma\n");
    printf("  create <testo>    - Crea una nuova card\n");
    printf("  help              - Mostra questo messaggio\n");
    printf("\n");
}