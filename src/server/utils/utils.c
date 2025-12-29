#include "utils.h"

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



