#include "core.h"
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include "../../shared/core/core.h"

Lavagna lav;
static int next_card_id =1 1;

void initialize_lavagna() {
    printf("\n========================================\n");
    printf("  INIZIALIZZAZIONE LAVAGNA KANBAN\n");
    printf("========================================\n\n");
    
    lav.id_lavagna = 1;
    lav.num_utenti = 0;
    lav.num_cards = 0;
    
    for(int i = 0; i < MAX_CARDS; i++) {
        lav.cards[i] = NULL;
    }
    
    for(int i = 0; i < MAX_CLIENTS; i++) {
        lav.lista_utenti[i].socket_fd = -1;
        lav.lista_utenti[i].port = 0;
        lav.lista_utenti[i].attivo = false;
        lav.lista_utenti[i].last_ping = 0;
    }
    
    create_initial_cards();
    show_lavagna();
}

void create_initial_cards() {
    const char* initial_tasks[INITIAL_CARDS] = {
        "Spesa",
        "Lavatrice",
        "Spazzare",
        "Implementare protocollo",
        "Debuggare progetto",
        "Ottimizzare lato server",
        "Aggiornare documentazione",
        "Irrobustire la sicurezza",
        "Implementare logging",
        "Revisionare codice"
    };
    
    printf("[INIT] Creazione %d card iniziali...\n", INITIAL_CARDS);
    for(int i = 0; i < INITIAL_CARDS; i++) {
        create_card(initial_tasks[i], 0);
    }
    printf("[INIT] Card iniziali create con successo\n\n");
}

Card* create_card(const char* text, int user_port) {
    if(lav.num_cards >= MAX_CARDS) {
        printf("[ERROR] Lavagna piena, impossibile creare nuova card\n");
        return NULL;
    }
    
    Card* new_card = (Card*)malloc(sizeof(Card));
    if(!new_card) {
        perror("malloc");
        return NULL;
    }
    
    new_card->id = next_card_id++;
    new_card->column = COL_TODO;
    strncpy(new_card->text, text, MAX_TEXT_LEN - 1);
    new_card->text[MAX_TEXT_LEN - 1] = '\0';
    new_card->user_port = user_port;
    new_card->last_update = time(NULL);
    
    lav.cards[lav.num_cards++] = new_card;
    
    return new_card;
}

void move_card(int card_id, ColumnType new_column) {
    Card* card = get_card_by_id(card_id);
    if(!card) {
        printf("[ERROR] Card %d non trovata\n", card_id);
        return;
    }
    
    const char* old_col = column_to_string(card->column);
    const char* new_col = column_to_string(new_column);
    
    printf("[MOVE_CARD] Card %d: %s -> %s\n", card_id, old_col, new_col);
    
    card->column = new_column;
    card->last_update = time(NULL);
    
    show_lavagna();
}

Card* get_card_by_id(int card_id) {
    for(int i = 0; i < lav.num_cards; i++) {
        if(lav.cards[i] && lav.cards[i]->id == card_id) {
            return lav.cards[i];
        }
    }
    return NULL;
}

void show_lavagna() {
    printf("\n╔════════════════════════════════════════════════════════════════════╗\n");
    printf("║                      STATO LAVAGNA KANBAN                          ║\n");
    printf("╠════════════════════════════════════════════════════════════════════╣\n");
    printf("║  Utenti attivi: %d                                               ║\n", lav.num_utenti);
    printf("╚════════════════════════════════════════════════════════════════════╝\n\n");
    
    // Conta card per colonna
    int todo_count = 0, doing_count = 0, done_count = 0;
    for(int i = 0; i < lav.num_cards; i++) {
        if(!lav.cards[i]) continue;
        switch(lav.cards[i]->column) {
            case COL_TODO: todo_count++; break;
            case COL_DOING: doing_count++; break;
            case COL_DONE: done_count++; break;
        }
    }
    
    printf("┌─────────────────────┬─────────────────────┬─────────────────────┐\n");
    printf("│   TODO (%2d)         │   DOING (%2d)        │   DONE (%2d)         │\n", 
           todo_count, doing_count, done_count);
    printf("├─────────────────────┼─────────────────────┼─────────────────────┤\n");
    
    // Trova numero massimo di righe necessarie
    int max_rows = todo_count;
    if(doing_count > max_rows) max_rows = doing_count;
    if(done_count > max_rows) max_rows = done_count;
    
    // Array per memorizzare le card per colonna
    Card* todo_cards[MAX_CARDS] = {NULL};
    Card* doing_cards[MAX_CARDS] = {NULL};
    Card* done_cards[MAX_CARDS] = {NULL};
    
    int todo_idx = 0, doing_idx = 0, done_idx = 0;
    for(int i = 0; i < lav.num_cards; i++) {
        if(!lav.cards[i]) continue;
        switch(lav.cards[i]->column) {
            case COL_TODO: 
                todo_cards[todo_idx++] = lav.cards[i];
                break;
            case COL_DOING:
                doing_cards[doing_idx++] = lav.cards[i];
                break;
            case COL_DONE:
                done_cards[done_idx++] = lav.cards[i];
                break;
        }
    }
    
    // Stampa le righe
    for(int row = 0; row < max_rows; row++) {
        printf("│");
        
        // Colonna TODO
        if(row < todo_count && todo_cards[row]) {
            char text[19];
            strncpy(text, todo_cards[row]->text, 18);
            text[18] = '\0';
            printf(" [%2d] %-14s │", todo_cards[row]->id, text);
        } else {
            printf("                     │");
        }
        
        // Colonna DOING
        if(row < doing_count && doing_cards[row]) {
            char text[19];
            strncpy(text, doing_cards[row]->text, 14);
            text[14] = '\0';
            printf(" [%2d] %-11s  │", doing_cards[row]->id, text);
        } else {
            printf("                     │");
        }
        
        // Colonna DONE
        if(row < done_count && done_cards[row]) {
            char text[19];
            strncpy(text, done_cards[row]->text, 14);
            text[14] = '\0';
            printf(" [%2d] %-11s  │", done_cards[row]->id, text);
        } else {
            printf("                     │");
        }
        
        printf("\n");
    }
    
    printf("└─────────────────────┴─────────────────────┴─────────────────────┘\n\n");
}

Utente* find_utente_by_port(int port) {
    for(int i = 0; i < MAX_CLIENTS; i++) {
        if(lav.lista_utenti[i].attivo && lav.lista_utenti[i].port == port) {
            return &lav.lista_utenti[i];
        }
    }
    return NULL;
}

Utente* find_utente_by_socket(int socket_fd) {
    for(int i = 0; i < MAX_CLIENTS; i++) {
        if(lav.lista_utenti[i].attivo && lav.lista_utenti[i].socket_fd == socket_fd) {
            return &lav.lista_utenti[i];
        }
    }
    return NULL;
}

int get_active_users_count() {
    return lav.num_utenti;
}

void get_active_users_list(int* ports, int* count) {
    *count = 0;
    for(int i = 0; i < MAX_CLIENTS; i++) {
        if(lav.lista_utenti[i].attivo) {
            ports[(*count)++] = lav.lista_utenti[i].port;
        }
    }
}

void handle_command(Message* msg) {
    // Parsing messaggio
    
    printf("[CMD] Ricevuto %s da porta %d\n", command_to_string(msg->type), msg->sender_port);
    
    switch(msg->type) {
        case CMD_HELLO:
            handle_hello(msg);
            break;
        case CMD_QUIT:
            handle_quit(msg);
            break;
        case CMD_CREATE_CARD:
            handle_create_card(msg);
            break;
        case CMD_ACK_CARD:
            handle_ack_card(msg);
            break;
        case CMD_CARD_DONE:
            handle_card_done(msg);
            break;
        case CMD_SHOW_LAVAGNA:
            handle_show_lavagna(msg);
            break;
        case CMD_PONG_LAVAGNA:
            handle_pong_lavagna(msg);
            break;
        default:
            printf("[WARN] Comando non gestito: %s\n", command_to_string(msg->type));
            break;
    }
}

void handle_hello(Message* msg) {

    // Verifica se l'utente è già registrato
    if(find_utente_by_port(msg->sender_port)) {
        printf("[WARN] Utente %d già registrato\n", msg->sender_port);
        send_to_client(event->client_fd, "ERR_ALREADY_REGISTERED\n", 24);
        return;
    }
    
    // Trova slot libero
    for(int i = 0; i < MAX_CLIENTS; i++) {
        if(!lav.lista_utenti[i].attivo) {
            lav.lista_utenti[i].attivo = true;
            lav.lista_utenti[i].port = msg->sender_port;
            lav.lista_utenti[i].socket_fd = msg->socket;
            lav.lista_utenti[i].last_ping = time(NULL);
            lav.num_utenti++;
            
            printf("[HELLO] Utente registrato: Porta %d, Totale utenti: %d\n", 
                   msg->sender_port, lav.num_utenti);
            
            send_to_client(msg->socket_fd, "ACK_HELLO\n", 10);
            
            // Verifica se possiamo iniziare ad assegnare card (PARI: almeno 2 utenti)
            check_and_send_available_cards();
            return;
        }
    }
    
    send_to_client(event->client_fd, "ERR_LAVAGNA_FULL\n", 17);
}

