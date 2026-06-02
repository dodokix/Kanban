#include "core.h"
#include "../net/net.h"
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

Lavagna lav;
static int next_card_id = 1;

void init_auction(){
    lav.auction.acks_received = 0;
    lav.auction.partecipating_users = 0;
    lav.auction.id = 0;
    lav.auction.active = false;
}

void initialize_lavagna() {
    printf("\n========================================\n");
    printf("  INIZIALIZZAZIONE LAVAGNA KANBAN\n");
    printf("========================================\n\n");
    
    lav.id_lavagna = 1;
    lav.num_utenti = 0;
    lav.n_free = 0;
    lav.num_cards = 0;
    lav.card_in_asta = 0;
    init_auction();
    
    for(int i = 0; i < MAX_CARDS; i++) {
        lav.cards[i] = NULL;
    }
    
    for(int i = 0; i < MAX_CLIENTS; i++) {
        lav.lista_utenti[i].socket_fd = -1;
        lav.lista_utenti[i].port = 0;
        lav.lista_utenti[i].attivo = false;
        lav.lista_utenti[i].occupato= false;
        lav.lista_utenti[i].last_pong = 0;
        lav.lista_utenti[i].ping_sent = 0;
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
    new_card->user_port = user_port;
    new_card->last_update = time(NULL);
    strncpy(new_card->text, text, MAX_TEXT_LEN - 1);
    new_card->text[MAX_TEXT_LEN - 1] = '\0';
    
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
    
    
    // Array per memorizzare le card per colonna
    Card* todo_cards[MAX_CARDS];
    Card* doing_cards[MAX_CARDS];
    Card* done_cards[MAX_CARDS];
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
    
    // Trova numero massimo di righe necessarie
    int max_rows = todo_count;
    if(doing_count > max_rows) max_rows = doing_count;
    if(done_count > max_rows) max_rows = done_count;

    // Stampa le righe
    for(int row = 0; row < max_rows; row++) {
        printf("│");
        
        // Colonna TODO
        if(row < todo_count) {
            char text[15];
            strncpy(text, todo_cards[row]->text, 14);
            text[14] = '\0';
            printf(" [%2d] %-14s │", todo_cards[row]->id, text);
        } else {
            printf("                     │");
        }
        
        // Colonna DOING
        if(row < doing_count) {
            char text[15];
            strncpy(text, doing_cards[row]->text, 14);
            text[14] = '\0';
            printf(" [%2d] %-14s  │", doing_cards[row]->id, text);
        } else {
            printf("                     │");
        }
        
        // Colonna DONE
        if(row < done_count) {
            char text[15];
            strncpy(text, done_cards[row]->text, 14);
            text[14] = '\0';
            printf(" [%2d] %-14s  │", done_cards[row]->id, text);
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
        case CMD_READY:
            handle_ready(msg);
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
    else if(strncmp(msg->text, "quit", strlen("quit")) == 0){
        close_net();
        exit(0);
    } 
    else if(strncmp(msg->text, "show", strlen("show")) == 0) show_lavagna();
    else if(strncmp(msg->text, "help", strlen("help")) == 0) show_help();
    else{
        printf("[WARN] Comando constole non riconosciuto: '%s'\n", msg->text);
    }
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
            lav.lista_utenti[i].last_pong = time(NULL);
            lav.lista_utenti[i].ping_sent = 0;
            lav.num_utenti++;
            
            printf("[HELLO] Utente registrato, totale utenti: %d\n", lav.num_utenti);
            
            Message ack;
            memset(&ack, 0, sizeof(Message));
            ack.type = CMD_HELLO;
            strncpy(ack.text, "ACK", sizeof(ack.text)-1);
            send_to_client(&ack, socket_fd);
            
            // Verifica se possiamo iniziare ad assegnare card
            check_and_send_available_cards();
            return;
        }
    }
}

void handle_quit(Message* msg){
    Utente* u = find_utente_by_port(msg->sender_port);
    if(!u){
        printf("[WARN] QUIT da utente sconosciuto: %d\n", msg->sender_port);
        return;
    }

    u->attivo = false;
    u->occupato = false;
    u->socket_fd = -1;
    lav.num_utenti--;
    printf("[QUIT] Utente %d disconnesso.\n", msg->sender_port);

    for(int i = 0; i<lav.num_cards; ++i){
        if(lav.cards[i] &&
            lav.cards[i]->column == COL_DOING &&
            lav.cards[i]->user_port == msg->sender_port ){
                move_card(lav.cards[i]->id, COL_TODO);
                lav.cards[i]->user_port = 0;
        }
    }
    
    check_and_send_available_cards();
}

void handle_create_card(Message* msg) {
    printf("[CREATE_CARD] Nuova card da utente %d: %s\n", msg->sender_port, msg->text);
    create_card(msg->text, msg->sender_port);
    show_lavagna();
    check_and_send_available_cards();
}

void handle_ack_card(Message* msg){
    // Utente vince l'asta e prende la card
    Card* c = get_card_by_id(msg->card_id);
    if(c && c->column == COL_TODO) {
        c->user_port = msg->sender_port;
        lav.card_in_asta = 0;
        
        move_card(c->id, COL_DOING);
        
        Utente* u = find_utente_by_port(msg->sender_port);
        if(u) {
            u->occupato = true;
            printf("[INFO] Utente %d ora occupato su card %d\n", msg->sender_port, msg->card_id);
        }
        init_auction();
    }
}


void handle_ready(Message* msg){
    lav.auction.acks_received++;
    printf("[AUCTION %d] Client %d ready. %d/%d\n", lav.auction.id, msg->sender_port, lav.auction.acks_received, lav.auction.partecipating_users);
    
    if(lav.auction.acks_received == lav.auction.partecipating_users){
        printf("[AUCTION] all participants ready! go!\n");

        Message start_msg;
        memset(&start_msg, 0, sizeof(Message));
        start_msg.type = CMD_START_AUCTION;
        start_msg.card_id = msg->card_id;
        
        for(int i = 0; i<lav.num_utenti; ++i){
            send_to_client(&start_msg, lav.lista_utenti[i].socket_fd);
        }
    }
}



void handle_card_done(Message* msg) {
    // Utente completa il task
    Card* c = get_card_by_id(msg->card_id);
    if(c && c->column == COL_DOING && c->user_port == msg->sender_port) {
        move_card(c->id, COL_DONE);

        Utente* u = find_utente_by_port(msg->sender_port);
        if(u){
            u->occupato = false;
            printf("[CARD_DONE] Utente %d completa card %d\n", msg->sender_port, msg->card_id);
        }
        check_and_send_available_cards();
    }
    else{
        printf("[WARN] CARD_DONE ignorato: card %d non in DOING o utente errato\n",
        msg->card_id);
    }
}

void handle_pong_lavagna(Message* msg) {
    Utente* u = find_utente_by_port(msg->sender_port);
    if(u){
        u->last_pong = time(NULL);
        u->ping_sent=0;
        printf("[PONG] Utente %d ha risposto al ping\n", msg->sender_port);
    }
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

    int free_ports[MAX_USERS];
    int n_free = 0;
    
    for(int i = 0; i < MAX_USERS; ++i){
        if(lav.lista_utenti[i].attivo && !lav.lista_utenti[i].occupato){
            free_ports[n_free++] = lav.lista_utenti[i].port;
        }
    }

    if(n_free < 2){
        printf("[INFO] Utenti liberi insufficienti (%d) per avviare un'asta. \n", n_free);
        return;
    }

    printf("\n[AVAILABLE CARD] Invio card %d a %d utenti\n", card->id, n_free);

    for(int i = 0; i < n_free; ++i){
        Utente* utente = find_utente_by_port(free_ports[i]);
        if(!utente) continue;

        Message msg;
        memset(&msg, 0, sizeof(Message));
        msg.type = CMD_AVAILABLE_CARD;
        msg.sender_port = SERVER_PORT;
        msg.card_id = card->id;
        msg.column = card->column;
        strncpy(msg.text, card->text, MAX_TEXT_LEN - 1);
        
        int k = 0;
        for(int j = 0; j < n_free; ++j){
            if(free_ports[j] != free_ports[i])
                msg.user_list[k++] = free_ports[j];
        }
        msg.num_users = k;

        send_to_client(&msg, utente->socket_fd);
    }
}


void check_and_send_available_cards() {
    if(lav.num_utenti < 2) return;
    if(lav.card_in_asta != 0) return;

    for(int i=0; i<lav.num_cards; i++) {
        if(lav.cards[i] && lav.cards[i]->column == COL_TODO) {
            lav.card_in_asta = lav.cards[i]->id;
            lav.auction.partecipating_users = lav.num_utenti;
            broadcast_available_cards();
            break;
        }
    }
}

void close_client(Utente* u){
    close(u->socket_fd);
    u->attivo = false;
    u->occupato = false;
    u->port = 0;
    u->socket_fd = -1;
    u->ping_sent = 0;
    lav.num_utenti--;
}

void check_ping_timeouts(){
    time_t now = time(NULL);

    for(int i = 0; i < lav.num_cards; ++i){
        Card* card = lav.cards[i];
        if(!card || card->column != COL_DOING) continue;

        Utente* utente = find_utente_by_port(card->user_port);
        if(!utente) continue;

        if(utente->ping_sent > 0 &&
           difftime(now, utente->ping_sent) > PONG_TIMEOUT) {
            printf("[TIMEOUT] Utente %d non risponde al PING: "
                   "card %d rimessa in TODO\n", utente->port, card->id);
            card->column      = COL_TODO;
            card->user_port   = 0;

            close_client(utente);
            
            if(lav.num_utenti < 2)
                lav.card_in_asta = 0;

            show_lavagna();
            continue;
        }
        
        if(utente->ping_sent == 0 &&
            difftime(now, card->last_update) > PING_TIMEOUT) {
                Message ping_msg;
                memset(&ping_msg, 0, sizeof(Message));
                ping_msg.type        = CMD_PING_USER;
                ping_msg.sender_port = SERVER_PORT;
                ping_msg.card_id     = card->id;
                
                send_to_client(&ping_msg, utente->socket_fd);
                utente->ping_sent = now; 
                printf("[PING] Inviato a utente %d per card %d\n",
                    utente->port, card->id);
                }
            }
        check_and_send_available_cards();
}