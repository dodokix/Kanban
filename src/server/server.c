#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include "include/lavagna.h"
#include <errno.h>
#include <time.h>

void cmd_hello(int client_fd, char *args, Lavagna *lav);
void cmd_quit(int client_fd, Lavagna *lav);
void cmd_create_card(const char* buffer, int client_fd, Lavagna *lav);
void cmd_show_lavagna(Lavagna *lav);
void first_cards(Lavagna *lav);

int main(int argc, char*argv[]){
    Lavagna *lav = initialize_lav();
    int sv_fd, max_sd, sd, activity;
    int new_sock, *socket_thread;
    int client_socket[MAX_CLIENTS] = {0};
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    fd_set readfds;
    char buffer[BUFFER_SIZE];


    if((sv_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
        perror("Socket failed!");
        exit(EXIT_FAILURE);
    }

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = INADDR_ANY;

    if((bind(sv_fd, (struct sockaddr*)&address, sizeof(address))) < 0 ){
        perror("Bind failed!");
        close(sv_fd);
        exit(EXIT_FAILURE);
    }

    if((listen(sv_fd, 4)) < 0){
        perror("Listen failed!");
        close(sv_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    while(1){

        FD_ZERO(&readfds);

        FD_SET(sv_fd, &readfds);
        max_sd = sv_fd;

        for(int i = 0; i<MAX_CLIENTS; ++i){
            sd = client_socket[i];
            if(sd > 0){
                FD_SET(sd, &readfds);
            }
            if(sd > max_sd){
                max_sd = sd;
            }
        }

        activity = select(max_sd + 1, &readfds, NULL,NULL,NULL);

        if((activity < 0) && (errno != EINTR)) {
            perror("select failed!\n");
        }

        if(FD_ISSET(sv_fd, &readfds)){

            if((new_sock = accept(sv_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0){
                perror("Accept failed!\n");
                exit(EXIT_FAILURE);
            }
        
            printf("Nuova connessione stabilita con client! \n");

            for(int i = 0; i < MAX_CLIENTS; ++i){
                if(client_socket[i] == 0){
                    client_socket[i] = new_sock;
                    printf("aggiungo nuovo client alla lista,\n");
                    break;
            
                }
            }
        }


        for (int i = 0; i < MAX_CLIENTS; i++) {
            sd = client_socket[i];
            if (FD_ISSET(sd, &readfds)) {
                int valread = read(sd, buffer, BUFFER_SIZE - 1);
                if (valread <= 0) {
                    // Il client ha chiuso la connessione
                    printf("Client disconnesso (socket fd: %d)\n", sd);
                    //GESTIONE CARD IN DOING.
                    close(sd);
                    client_socket[i] = 0;
                } else {
                    buffer[valread] = '\0';
                    char *cmd = strtok(buffer, " ");
                    char *args = strtok(NULL, "");

                    if(cmd != NULL && strcmp(cmd, "HELLO") == 0){
                        cmd_hello(sd, args, lav);
                    }
                    else if(cmd != NULL && strcmp(cmd, "QUIT") == 0){
                        cmd_quit(sd, lav);
                        FD_CLR(sd, &readfds);
                        close(sd);
                        client_socket[i] = 0;
                        printf("connessione chiusa correttamente dopo QUIT.\n");
                    }

                }
            }
        }
    }

    return 0;
}

















void cmd_hello(int client_fd, char* args, Lavagna *lav){
    int porta_utente;
    printf("il client ha inviato HELLO! \n");

    if(args == NULL || strlen(args) == 0){
        perror("argomenti passati errati\n");
        return;
    }

    porta_utente = atoi(args);
    if(porta_utente < 5679){
        perror("porta passata non valida\n");
        return;
    }

    if(lav->user_counter >= MAX_CLIENTS){
        perror("numero di utenti massimo raggiunto\n");
        return;
    }

    int slot_trovato = -1;
    for(int i = 0; i < MAX_CLIENTS; ++i){
        if(!lav->lista_utenti[i].attivo){
            lav->lista_utenti[i].socket_fd = client_fd;
            lav->lista_utenti[i].port = porta_utente;
            lav->lista_utenti[i].attivo = 1;
            lav->user_counter++;
            slot_trovato = i;
            break;
        }
    }

    if(slot_trovato != -1){
        printf("utente registrato! porta: %d\n", porta_utente);
        char risposta[100];
        sprintf(risposta, "OK HELLO, registrato con porta: %d\n", porta_utente);
        send(client_fd, risposta, strlen(risposta), 0);
    }
}

void cmd_quit(int client_fd, Lavagna *lav){

    int porta_utente = get_port_from_socket(client_fd, lav);
    if(porta_utente <= 5679){
        perror("porta non valida\n");
        return;
    }

    int trovato = -1;
    for(int i = 0; i < MAX_CLIENTS; ++i){
        if(lav->lista_utenti[i].attivo && lav->lista_utenti[i].port == porta_utente){

            //GESTIONE CARD IN DOING!!  

            trovato = i;
            lav->lista_utenti[i].attivo = 0;
            lav->lista_utenti[i].port = 0;
            lav->lista_utenti[i].socket_fd = 0;
            lav->user_counter--;
        }
    }

    if(trovato == -1){
        perror("l'utente non si e registrato\n");
        return;
    }

    char risposta[100];
    sprintf(risposta, "l'utente ha cancellato la sua registrazione con successo\n");
    send(client_fd, risposta, strlen(risposta), 0);


}




void cmd_move_card(int card_id, CardStatus nuovo_status, Lavagna *lav){

    int i_card = -1;
    for(int i = 0; i < MAX_CARDS; ++i){
        if(lav->lista_card[i]->id == card_id){
            i_card = i;
        }
    }

    if(i_card == -1){
        printf("card non trovata! \n");
        return;
    }

    lav->lista_card[i_card]->colonna = nuovo_status;
    return;
}

const char* stato_to_string(CardStatus s){
    switch(s){
        case TO_DO: return "---[TO_DO]---";
        case DOING: return "---[DOING]---";
        case DONE: return "---[DONE]---";
        default: return "---[UNKNOWN]---";
    }
}

void cmd_show_lavagna(Lavagna *lav){

    printf("\n=======================================\n");
    printf("             LAVAGNA ID: %d\n", lav->id_lavagna);
    printf("=======================================\n");

    for(CardStatus s = TO_DO; s <= DONE; ++s){
        printf("%s", stato_to_string(s));
        for(int i = 0; i < lav->num_card; ++i){
            if(lav->lista_card[i]->colonna == s){
                printf("(User: %d) [%d]:  %-20s \n", 
                lav->lista_card[i]->id,
                lav->lista_card[i]->porta_utente,
                lav->lista_card[i]->testo 
                );
            }
        }
    }

    printf("\n=======================================\n");

    
}




