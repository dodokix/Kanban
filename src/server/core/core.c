#include "core.h"
#include "../net/net.h"
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include "../../shared/core/core.h"

Lavagna lav;
static int next_card_id = 1;

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
        "Protocollo",
        "Debugging",
        "Ottimizzazione",
        "Documentazione",
        "Sicurezza",
        "Logging",
        "Code review"
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

void move_card(int card_id, CardStatus new_column) {
    Card* card = get_card_by_id(card_id);
    if(!card) {
        printf("[ERROR] Card %d non trovata\n", card_id);
        return;
    }
    
    
    printf("[MOVE_CARD] Card %d: %s -> %s\n",
            card_id, column_to_string(card->column),column_to_string(new_column));
    
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

int get_active_users_list(int* ports) {
    int count = 0;
    for(int i = 0; i < MAX_CLIENTS; i++) {
        if(lav.lista_utenti[i].attivo) {
            ports[count++] = lav.lista_utenti[i].port;
        }
    }
    return count;
}

void show_help(){
    printf("Comandi disponibili: \n");
    printf("  create <text>          - Crea una card\n");
    printf("  show                   - Mostra stato lavagna\n");
    printf("  quit                   - Termina server\n");
    printf("  help                   - Mostra questo messaggio\n\n");
    return;
}

void handle_command(Message* msg, int socket_fd) {

    switch(msg->type) {
        case CMD_CONSOLE:
            handle_console(msg);
            break;
        case CMD_HELLO:
            handle_hello(msg, socket_fd);
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
            show_lavagna();
            break;
        case CMD_PONG_LAVAGNA:
            handle_pong_lavagna(msg);
            break;
        default:
            printf("[WARN] Comando non gestito: %s\n", command_to_string(msg->type));
            break;
    }
}

void handle_console(Message* msg){
    if(strncmp(msg->text, "create", strlen("create")) == 0){
        create_card(msg->text + strlen("create "), SERVER_PORT);    
        show_lavagna();
        check_and_send_available_cards();
    }
    if(strncmp(msg->text, "quit", strlen("quit")) == 0){
        close_net();
        exit(0);
    } 
    if(strncmp(msg->text, "show", strlen("show")) == 0) show_lavagna();
    if(strncmp(msg->text, "help", strlen("help")) == 0) show_help();
}

void handle_hello(Message* msg, int socket_fd) {

    // Verifica se l'utente è già registrato
    if(find_utente_by_port(msg->sender_port)) {
        printf("[WARN] Utente %d già registrato\n", msg->sender_port);
        return;
    }
    
    // Trova slot libero
    for(int i = 0; i < MAX_CLIENTS; i++) {
        if(!lav.lista_utenti[i].attivo) {
            lav.lista_utenti[i].attivo = true;
            lav.lista_utenti[i].port = msg->sender_port;
            lav.lista_utenti[i].socket_fd = socket_fd;
            lav.lista_utenti[i].last_ping = time(NULL);
            lav.num_utenti++;
            
            printf("[HELLO] Utente registrato: Porta %d, Totale utenti: %d\n", 
                   msg->sender_port, lav.num_utenti);
            
            Message ack;
            memset(&ack, 0, sizeof(Message));
            ack.type = CMD_HELLO;
            strcpy(ack.text, "ACK");
            send_to_client(&ack, socket_fd);
            
            // Verifica se possiamo iniziare ad assegnare card
            check_and_send_available_cards();
            return;
        }
    }
}

void handle_quit(Message* msg){
    Utente* u = find_utente_by_port(msg->sender_port);
    if(u){
        u->attivo = false;
        u->socket_fd = -1;
        lav.num_utenti--;
        printf("[INFO] Utente %d disconnesso.\n", msg->sender_port);

        for(int i = 0; i<lav.num_cards; ++i){
            if(lav.cards[i] &&
                lav.cards[i]->column == COL_DOING &&
                lav.cards[i]->user_port == msg->sender_port )
            move_card(lav.cards[i]->id, COL_TODO);
            lav.cards[i]->user_port = 0;
        }
    }
    check_and_send_available_cards();
}

void handle_create_card(Message* msg) {
    printf("[CREATE_CARD] Nuova card: %s\n", msg->text);
    create_card(msg->text, msg->sender_port);
    show_lavagna();
    check_and_send_available_cards();
}

void handle_ack_card(Message* msg){
    // Utente vince l'asta e prende la card
    Card* c = get_card_by_id(msg->card_id);
    if(c && c->column == COL_TODO) {
        c->user_port = msg->sender_port;
        move_card(c->id, COL_DOING);
    }
}

void handle_card_done(Message* msg) {
    // Utente completa il task
    Card* c = get_card_by_id(msg->card_id);
    if(c && c->column == COL_DOING && c->user_port == msg->sender_port) {
        move_card(c->id, COL_DONE);
        printf("[CARD_DONE] Utente %d completa card %d\n", msg->sender_port, msg->card_id);
        check_and_send_available_cards(); // Proponi la prossima
    }
}

void handle_pong_lavagna(Message* msg) {
    Utente* u = find_utente_by_port(msg->sender_port);
    if(u) u->last_ping = time(NULL);
}

void broadcast_available_cards(){
    Card* card = NULL;
    for(int i = 0; i < lav.num_cards; ++i){
        if(lav.cards[i] && lav.cards[i]->column == COL_TODO){
            card = lav.cards[i];
            break;
        }
    }

    if(!card){
        printf("[INFO]: Nessuna card in TODO disponibile.\n");
        return;
    }

    int user_ports[MAX_USERS];
    int num_users = get_active_users_list(user_ports);

    if(num_users < 2){
        return;
    }

    printf("\n[AVAILABLE CARD] Invio card %d a %d utenti\n", card->id, num_users);

    for(int i = 0; i < num_users; ++i){
        Utente* utente = find_utente_by_port(user_ports[i]);
        if(!utente) continue;

        Message msg;
        memset(&msg, 0, sizeof(Message));
        msg.type = CMD_AVAILABLE_CARD;
        msg.sender_port = SERVER_PORT;
        msg.card_id = card->id;
        strncpy(msg.text, card->text, MAX_TEXT_LEN - 1);
        msg.column = card->column;
        msg.num_users = num_users;

        for(int j = 0; j < num_users; ++j){
            msg.user_list[j] = user_ports[j];
        }

        send_to_client(&msg, utente->socket_fd);
    }
}


void check_and_send_available_cards() {
    
    if(lav.num_utenti < 2) {
        return;
    }
    
    // Verifica se c'è almeno una card in TODO
    bool has_todo = false;
    for(int i = 0; i < lav.num_cards; i++) {
        if(lav.cards[i] && lav.cards[i]->column == COL_TODO) {
            has_todo = true;
            break;
        }
    }
    
    if(has_todo) {
        broadcast_available_cards();
    }
}

void check_ping_timeouts(){
    time_t now = time(NULL);

    for(int i = 0; i < lav.num_cards; ++i){
        Card* card = lav.cards[i];
        if(!card || card->column != COL_DOING) continue;

        if(difftime(now, card->last_update) > PING_TIMEOUT){
            Utente* utente = find_utente_by_port(card->user_port);
            if(!utente) continue;

            Message ping_msg;
            memset(&ping_msg, 0, sizeof(Message));
            ping_msg.type = CMD_PING_USER;
            ping_msg.card_id = card->id;

            send_to_client(&ping_msg, utente->socket_fd);
            printf("[PING]: inviato ad utente %d per card %d\n", utente->port, card->id);
            
            if(difftime(now, utente->last_ping) > PONG_TIMEOUT){
                printf("[TIMEOUT] Utente %d non ha risposto al PING, card %d rimessa in TODO\n",
                       utente->port, card->id);
                card->column = COL_TODO;
                card->user_port = 0;
                show_lavagna();
                check_and_send_available_cards(); 
            }
        }
    }
}