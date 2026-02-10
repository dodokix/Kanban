#ifndef SERVER_CORE_H
#define SERVER_CORE_H

#include <time.h>
#include <stdbool.h>
#include "../../shared/core/core.h"
#include "../../shared/protocol/protocol.h"
#include "../../shared/constants/core_constants.h"
#include "../../shared/constants/net_constants.h"

/* Struttura utente registrato */
typedef struct {
    int socket_fd;
    int port;
    bool attivo;
    time_t last_ping;
} Utente;

/* Struttura lavagna */
typedef struct {
    int id_lavagna;
    Utente lista_utenti[MAX_CLIENTS];
    int num_utenti;
    Card* cards[MAX_CARDS];
    int num_cards;
} Lavagna;

/* Funzioni di inizializzazione */
void initialize_lavagna();
void create_initial_cards();

/* Funzioni di gestione card */
Card* create_card(const char* text, int user_port);
void move_card(int card_id, CardStatus new_column);
Card* get_card_by_id(int card_id);
void show_lavagna();

/* Funzioni di gestione utenti */
Utente* find_utente_by_port(int port);
Utente* find_utente_by_socket(int socket_fd);
int get_active_users_count();
void get_active_users_list(int* ports, int* count);

/* Handler comando principale */
void handle_command(Message* event);

/* Handler comandi specifici */
void handle_hello(Message* event);
void handle_quit(Message* event);
void handle_create_card(Message* event);
void handle_ack_card(Message* event);
void handle_card_done(Message* event);
void handle_pong_lavagna(Message* event);

/* Funzioni per matricola PARI */
void send_available_card_to_all();
void check_and_send_available_cards();

/* Sistema PING */
void check_ping_timeouts();

#endif
