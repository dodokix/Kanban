
#include <time.h>
#include <stdbool.h>
#include "../../shared/core/core.h"

#define MAX_CLIENTS 4
#define MAX_CARDS 10

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


int get_port_from_socket(int client_fd, Lavagna *lav);
void cmd_create_card(const char* buffer, int client_fd, Lavagna *lav);
void first_cards(Lavagna *lav);
void initialize_lav();
void handle_command(ServerEvent* event);
