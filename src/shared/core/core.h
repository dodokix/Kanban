#ifndef SHARED_CORE_CORE_H
#define SHARED_CORE_CORE_H

#include <stdint.h>

/* Definizione tipi di colonna */
typedef enum {
    COL_TODO,
    COL_DOING,
    COL_DONE
} ColumnType;

/* Definizione tipi di comando */
typedef enum {
    CMD_HELLO,              // Registrazione utente
    CMD_QUIT,               // Disconnessione utente
    CMD_CREATE_CARD,        // Creazione nuova card
    CMD_AVAILABLE_CARD,     // Lavagna notifica card disponibile 
    CMD_CHOOSE_USER,        // Utenti si scambiano costi 
    CMD_ACK_CARD,           // Utente conferma presa in carico
    CMD_CARD_DONE,          // Utente notifica completamento
    CMD_MOVE_CARD,          // Lavagna sposta card
    CMD_PING_USER,          // Lavagna verifica stato utente
    CMD_PONG_LAVAGNA,       // Utente risponde a ping
    CMD_SEND_USER_LIST,     // Lavagna invia lista utenti
    CMD_SHOW_LAVAGNA,       // Mostra stato lavagna
    CMD_UNKNOWN             // Comando sconosciuto
} CommandType;


#endif
