#include "core.h"
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include "../../shared/core/core.h"

Lavagna lav;

typedef void (*command_handler)(ServerEvent* event);

void hello_handler(ServerEvent* event);

typedef struct{
    char* name;
    command_handler handler;
}command_entry;

command_entry cmd_table[] = {
    {"HELLO", hello_handler},
    {NULL , NULL}
};

void initialize_lav(){
    lav.id_lavagna = 1;
    for(int i = 0; i<MAX_CARDS; ++i){
        lav.lista_card[i] = 0;
    }
    for(int i = 0; i<MAX_CLIENTS; ++i){
        lav.lista_clients[i].attivo = false;
    }

    lav.num_card = 0;
    lav.user_counter = 0;
}
void handle_command(ServerEvent* event){
    for(int i = 0; cmd_table[i].name != NULL; ++i){
        if(strncmp(cmd_table[i].name, event->buffer, strlen(cmd_table[i].name)) == 0){
            cmd_table[i].handler(event);
            return;
        }
    }
}

void hello_handler(ServerEvent* event){
    for(int i = 0; i<MAX_CLIENTS; ++i){
        if(lav.lista_clients[i].attivo != false)
            continue;
        lav.lista_clients[i].attivo = true;
        lav.lista_clients[i].port = event->port;
        lav.lista_clients[i].socket_fd = event->client_fd;
        lav.user_counter++;
        printf("Utente registrato: FD %d, Porta P2P %d. Totale utenti: %d\n", 
                            event->client_fd, event->port, lav.user_counter); 

        send(event->client_fd, "ACK_HELLO\n", strlen("ACK_HELLO\n"), 0);
        return;
    }

    send(event->client_fd, "ERR_HELLO\n", strlen("ERR_HELLO\n"), 0);
}




// int get_port_from_socket(int client_fd, Lavagna *lav){
//     for(int i = 0; i < MAX_CLIENTS; ++i){
//         if(lav->lista_clients[i].attivo && lav->lista_clients[i].socket_fd == client_fd){
//             return lav->lista_clients[i].port;
//         }
//     }
//     return -1;
// }

// void cmd_create_card(const char* buffer, int client_port, Lavagna *lav){
//     Card *new_card = malloc(sizeof(Card));

//     if(new_card == NULL){
//         perror("errore allocazione memeoria! \n");
//         return ;
//     }

//     new_card->id = lav->num_card++;
//     new_card->colonna = TO_DO;
//     if(client_fd == PORT){
//         new_card->porta_utente = PORT;

//     }
//     else{
//         new_card->porta_utente = get_port_from_socket(client_fd, lav);
//     }
//     strncpy(new_card->testo, buffer, sizeof(new_card->testo) - 1);
//     new_card->testo[sizeof(new_card->testo) - 1] = '\0';
//     new_card->last_update = time(NULL);

//     int aggiunta = -1;
//     for(int i = 0; i < MAX_CARDS; ++i){
//         if(!lav->lista_card[i]){
//             lav->lista_card[i] = new_card;
//             aggiunta = 1;
//             break;
//         }
//     }

//     if(!aggiunta){
//         printf("Errore: lavagna piena, impossibile aggiungere la card. \n");
//         free(new_card);
//         lav->num_card--;
//     }
// }

// void first_cards(Lavagna *lav){

//     const char *arr_buf[INITIAL_CARDS] = {
//         "task1", "task2","task3","task4","task5",
//         "task6","task7","task8","task9","task10", 
//     };

//     for(int i = 0; i < INITIAL_CARDS; ++i){
//         cmd_create_card(arr_buf[i], PORT, lav);
//     }
//     return;
    
// }