#include "protocol.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

const char* command_to_string(CommandType cmd) {
    switch(cmd) {
        case CMD_HELLO: return "HELLO";
        case CMD_QUIT: return "QUIT";
        case CMD_CREATE_CARD: return "CREATE_CARD";
        case CMD_AVAILABLE_CARD: return "AVAILABLE_CARD";
        case CMD_READY: return "CMD_READY";
        case CMD_START_AUCTION: return "START_AUCTION";
        case CMD_CHOOSE_USER: return "CHOOSE_USER";
        case CMD_ACK_CARD: return "ACK_CARD";
        case CMD_CARD_DONE: return "CARD_DONE";
        case CMD_MOVE_CARD: return "MOVE_CARD";
        case CMD_PING_USER: return "PING_USER";
        case CMD_PONG_LAVAGNA: return "PONG_LAVAGNA";
        case CMD_SEND_USER_LIST: return "SEND_USER_LIST";
        case CMD_SHOW_LAVAGNA: return "SHOW_LAVAGNA";
        default: return "UNKNOWN";
    }
}

CommandType string_to_command(const char* str) {
    if(strcmp(str, "HELLO") == 0) return CMD_HELLO;
    if(strcmp(str, "QUIT") == 0) return CMD_QUIT;
    if(strcmp(str, "CREATE_CARD") == 0) return CMD_CREATE_CARD;
    if(strcmp(str, "AVAILABLE_CARD") == 0) return CMD_AVAILABLE_CARD;
    if(strcmp(str, "CMD_READY") == 0) return CMD_READY;
    if(strcmp(str, "START_AUCTION") == 0) return CMD_START_AUCTION;
    if(strcmp(str, "CHOOSE_USER") == 0) return CMD_CHOOSE_USER;
    if(strcmp(str, "ACK_CARD") == 0) return CMD_ACK_CARD;
    if(strcmp(str, "CARD_DONE") == 0) return CMD_CARD_DONE;
    if(strcmp(str, "MOVE_CARD") == 0) return CMD_MOVE_CARD;
    if(strcmp(str, "PING_USER") == 0) return CMD_PING_USER;
    if(strcmp(str, "PONG_LAVAGNA") == 0) return CMD_PONG_LAVAGNA;
    if(strcmp(str, "SEND_USER_LIST") == 0) return CMD_SEND_USER_LIST;
    if(strcmp(str, "SHOW_LAVAGNA") == 0) return CMD_SHOW_LAVAGNA;
    return CMD_UNKNOWN;
}

const char* column_to_string(CardStatus col) {
    switch(col) {
        case COL_TODO: return "TODO";
        case COL_DOING: return "DOING";
        case COL_DONE: return "DONE";
        default: return "UNKNOWN";
    }
}

CardStatus string_to_column(const char* str) {
    if(strcmp(str, "TODO") == 0) return COL_TODO;
    if(strcmp(str, "DOING") == 0) return COL_DOING;
    if(strcmp(str, "DONE") == 0) return COL_DONE;
    return COL_TODO; // Default
}

/* Serializzazione messaggio in formato testuale
 * Formato: COMANDO|sender_port|card_id|cost|num_users|user_list|text|column\n
 * I campi non utilizzati vengono settati a valori di default (0 o stringa vuota)
 */
int serialize_message(const Message* msg, char* buffer, int buffer_size) {
    char user_list_str[512] = "";
    
    // Serializza la lista utenti
    if(msg->num_users > 0) {
        for(int i = 0; i < msg->num_users; i++) {
            char temp[16];
            snprintf(temp, sizeof(temp), "%d", msg->user_list[i]);
            if(i > 0) strcat(user_list_str, ",");
            strcat(user_list_str, temp);
        }
    }
    
    // Costruisco il messaggio
    int len = snprintf(buffer, buffer_size, 
                      "%s|%d|%d|%d|%d|%s|%s|%s\n",
                      command_to_string(msg->type),
                      msg->sender_port,
                      msg->card_id,
                      msg->cost,
                      msg->num_users,
                      user_list_str,
                      msg->text,
                      column_to_string(msg->column));
    
    if(len >= buffer_size) {
        return -1; // Buffer troppo piccolo
    }
    
    return len;
}

/* Deserializzazione messaggio da formato testuale */
int deserialize_message(const char* buffer, Message* msg) {
    char cmd_str[32] = "";
    char user_list_str[512] = "";
    char col_str[16] = "";
    
    // Inizializza struttura a zero
    memset(msg, 0, sizeof(Message));
    
    // Creiamo una copia del buffer per poterlo "spezzare" in sicurezza
    char temp_buffer[1024];
    strncpy(temp_buffer, buffer, sizeof(temp_buffer) - 1);
    temp_buffer[sizeof(temp_buffer) - 1] = '\0';
    
    // Rimuove il \n finale se presente
    char *newline = strchr(temp_buffer, '\n');
    if (newline) {
        *newline = '\0';
    }
    
    char *ptr = temp_buffer;
    char *token;
    int field = 0;
    
    // Ciclo di parsing: cerchiamo il pipe '|' e separiamo i campi
    while (ptr != NULL && field < 8) {
        token = ptr;
        ptr = strchr(ptr, '|'); 
        if (ptr != NULL) {
            *ptr = '\0'; // Inserisce un terminatore di stringa al posto del '|'
            ptr++;       // Sposta il puntatore al carattere successivo
        }
        
        // Assegna il token estratto al campo corrispondente
        switch(field) {
            case 0: strncpy(cmd_str, token, sizeof(cmd_str) - 1); break;
            case 1: msg->sender_port = atoi(token); break;
            case 2: msg->card_id = atoi(token); break;
            case 3: msg->cost = atoi(token); break;
            case 4: msg->num_users = atoi(token); break;
            case 5: strncpy(user_list_str, token, sizeof(user_list_str) - 1); break;
            case 6: strncpy(msg->text, token, sizeof(msg->text) - 1); break;
            case 7: strncpy(col_str, token, sizeof(col_str) - 1); break;
        }
        field++;
    }
    
    // Se non abbiamo letto tutti e 8 i campi, il pacchetto è malformato
    if (field < 8) {
        return -1; 
    }
    
    msg->type = string_to_command(cmd_str);
    msg->column = string_to_column(col_str);
    
    // Parse della lista utenti
    if(msg->num_users > 0 && strlen(user_list_str) > 0) {
        char* u_token = strtok(user_list_str, ",");
        for(int i = 0; i < MAX_USERS && u_token != NULL; ++i){
            msg->user_list[i] = atoi(u_token);
            u_token = strtok(NULL, ",");
        }
    }
    
    return 0;
}