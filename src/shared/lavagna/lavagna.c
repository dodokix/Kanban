#include "lavagna.h"
#include <stdbool.h>
#include <unistd.h>



int get_port_from_socket(int client_fd, Lavagna *lav){
    for(int i = 0; i < MAX_CLIENTS; ++i){
        if(lav->lista_utenti[i].attivo && lav->lista_utenti[i].socket_fd == client_fd){
            return lav->lista_utenti[i].port;
        }
    }
    return -1;
}

void cmd_create_card(const char* buffer, int client_fd, Lavagna *lav){
    Card *new_card = malloc(sizeof(Card));

    if(new_card == NULL){
        perror("errore allocazione memeoria! \n");
        return ;
    }

    new_card->id = lav->num_card++;
    new_card->colonna = TO_DO;
    if(client_fd == PORT){
        new_card->porta_utente = PORT;

    }
    else{
        new_card->porta_utente = get_port_from_socket(client_fd, lav);
    }
    strncpy(new_card->testo, buffer, sizeof(new_card->testo) - 1);
    new_card->testo[sizeof(new_card->testo) - 1] = '\0';
    new_card->last_update = time(NULL);

    int aggiunta = -1;
    for(int i = 0; i < MAX_CARDS; ++i){
        if(!lav->lista_card[i]){
            lav->lista_card[i] = new_card;
            aggiunta = 1;
            break;
        }
    }

    if(!aggiunta){
        printf("Errore: lavagna piena, impossibile aggiungere la card. \n");
        free(new_card);
        lav->num_card--;
    }
}

void first_cards(Lavagna *lav){

    const char *arr_buf[INITIAL_CARDS] = {
        "task1", "task2","task3","task4","task5",
        "task6","task7","task8","task9","task10", 
    };

    for(int i = 0; i < INITIAL_CARDS; ++i){
        cmd_create_card(arr_buf[i], PORT, lav);
    }
    return;
    
}

Lavagna* initialize_lav(){
    Lavagna *lav = malloc(sizeof(Lavagna));
    memset(lav, 0, sizeof(Lavagna));
    lav->id_lavagna = 0;
    first_cards(lav);
    return lav;
}
