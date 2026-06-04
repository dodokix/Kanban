#ifndef CLIENT_CORE_H
#define CLIENT_CORE_H
#include "../../shared/protocol/protocol.h"

/*
manda il messaggio CMD_HELLO al server per la registrazione
*/
void send_hello();

/*
stampa il messaggio con i comandi disponibili per l'esecuzione
*/
void print_help();

/*
gestisce i messaggi dal terminale
*/
void handle_stdin();

/*
gestisce i messaggi dal server
*/
void handle_server_message(Message* msg);
/*
gestisce i messaggi dagli altri utenti durante un'asta
*/
void handle_peer_message(Message* msg, int peer_socket_fd);
/*
confronta i costi degli utenti per verificare chi e' il vincitore dell'asta
*/
void check_auction_result();



#endif