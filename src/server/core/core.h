#ifndef SERVER_CORE_H
#define SERVER_CORE_H

#include <time.h>
#include <stdbool.h>
#include "../../shared/core/core.h"
#include "../../shared/protocol/protocol.h"
#include "../../shared/constants/core_constants.h"
#include "../../shared/constants/net_constants.h"

/* 
    Struttura utente registrato 
*/
typedef struct {
    int socket_fd;
    int port;
    bool attivo;
    bool occupato;
    time_t last_pong;
    time_t ping_sent;
} Utente;

/*
    struttura che descrive l'attuale asta in corso
*/
typedef struct{
    int acks_received;
    int partecipating_users;
    int id;
    bool active;
}Server_AuctionState;

/* 
    Struttura lavagna 
*/
typedef struct {
    int id_lavagna;
    Utente lista_utenti[MAX_CLIENTS];
    int num_utenti;
    int n_free;
    Card* cards[MAX_CARDS];
    int num_cards;
    int card_in_asta;
    Server_AuctionState auction;
} Lavagna;

extern Lavagna lav;

// funzioni di inizializzazione 
void initialize_lavagna();

// handler comando principale 
void handle_command(Message* event, int socket_fd);

// Sistema PING 
void check_ping_timeouts();

#endif
