#ifndef SHARED_CORE_H
#define SHARED_CORE_H

#define CMD_BUFF_SIZE 1024
#include <stdint.h>
#include <time.h>

typedef struct {
    int client_fd;
    uint16_t port;
    char buffer[CMD_BUFF_SIZE];
} ServerEvent;

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

#endif