#ifndef CARD_H
#define CARD_H

#include "../server_constants.h"
typedef struct{
    int socket_fd;
    int port;
    bool attivo;
} Utente;

typedef struct {
    int id_lavagna;
    Utente lista_clients[MAX_CLIENTS];
    int user_counter;
    int num_card;
    Card *lista_card[MAX_CARDS];
} Lavagna ;

typedef enum{
    TO_DO,
    DOING,
    DONE
}CardStatus;

typedef struct{
    int id;
    CardStatus colonna;
    char testo[256];
    int porta_utente;
    time_t last_update;
} Card;

int get_port_from_socket(int client_fd, Lavagna *lav);
void cmd_create_card(const char* buffer, int client_fd, Lavagna *lav);
void first_cards(Lavagna *lav);
Lavagna* initialize_lav();
