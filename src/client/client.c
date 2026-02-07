#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "net/net.h"

#define BUFFER_SIZE 1024


int main(int argc, char *argv[]){
    if(setup_server_socket() != 0){
        printf("errore in setup_server_socket\n");
    }
    
    return 0;
}
