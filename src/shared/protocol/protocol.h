#ifndef SHARED_CORE_H
#define SHARED_CORE_H

#define MAX_TEXT_LEN 256
#define MAX_CLIENTS 10
#define MAX_CARDS 50
#define SERVER_PORT 5678
#define SERVER_ADDRESS "127.0.0.1"
#define BACKLOG 10
#define CMD_BUFF_SIZE 2048

#include <stdint.h>
#include <time.h>
#include "../constants/net_constants.h"

typedef enum{
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

typedef struct {
    CommandType type;
    int client_fd;
    uint16_t port;
    int card_id;
    int cost;
    int num_users;
    int user_list[MAX_CLIENTS];
    char buffer[CMD_BUFF_SIZE];
} Message;

typedef enum{
    TO_DO,
    DOING,
    DONE
}CardStatus;

typedef struct{
    int id;
    CardStatus colonna;
    char testo[MAX_TEXT_LEN];
    int porta_utente;
    time_t last_update;
} Card;

const char* command_to_string(CommandType cmd);
CommandType string_to_command(const char* str);
const char* column_to_string(ColumnType col);
ColumnType string_to_column(const char* str);

int serialize_message(const Message* msg, char* buffer, int buffer_size);
int deserialize_message(const char* buffer, Message* msg);

#endif