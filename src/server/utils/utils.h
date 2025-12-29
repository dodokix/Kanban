
#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include "../client_constants.h"
#include "../server_constants.h"

void cmd_hello(int client_fd, char *args, Lavagna *lav);
void cmd_quit(int client_fd, Lavagna *lav);
void cmd_create_card(const char* buffer, int client_fd, Lavagna *lav);
void cmd_show_lavagna(Lavagna *lav);
void first_cards(Lavagna *lav);