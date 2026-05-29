#ifndef SHARED_CORE_H
#define SHARED_CORE_H

#include <stdint.h>
#include <time.h>
#include "../core/core.h"
#include "../constants/net_constants.h"
#include "../constants/core_constants.h"

typedef struct {
    CommandType type;
    int sender_port;
    int card_id;
    int cost;
    int num_users;
    int user_list[MAX_CLIENTS];
    char text[CMD_BUFF_SIZE];
    CardStatus column;
} Message;

typedef struct{
    int id;
    CardStatus column;
    char text[MAX_TEXT_LEN];
    int user_port;
    time_t last_update;
} Card;

const char* command_to_string(CommandType cmd);
CommandType string_to_command(const char* str);

const char* column_to_string(CardStatus col);
CardStatus string_to_column(const char* str);

int serialize_message(const Message* msg, char* buffer, int buffer_size);
int deserialize_message(const char* buffer, Message* msg);

#endif