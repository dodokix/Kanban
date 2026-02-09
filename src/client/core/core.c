#include "core.h"
#include "net.h"
#include "protocol.h"

void send_hello() {
    Message msg;
    memset(&msg, 0, sizeof(Message));
    
    msg.type = CMD_HELLO;
    msg.sender_port = my_port;
    
    send_to_server(&msg);
    printf("[CMD] HELLO inviato alla lavagna\n");
}

void print_help() {
    printf("  quit              - Esci dal programma\n");
    printf("  create <testo>    - Crea una nuova card\n");
    printf("  status            - Mostra stato corrente\n");
    printf("  help              - Mostra questo messaggio\n");
    printf("\n");
}