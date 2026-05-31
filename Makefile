CC = gcc
CFLAGS = -Wall -Wextra -std=c17 -g 

SHARED_SRCS = src/shared/protocol/protocol.c
 
SERVER_SRCS = src/server/server.c \
              src/server/core/core.c \
              src/server/net/net.c \
              $(SHARED_SRCS)
 
CLIENT_SRCS = src/client/client.c \
              src/client/core/core.c \
              src/client/net/net.c \
              $(SHARED_SRCS)
 
all: lavagna utente
 
lavagna: $(SERVER_SRCS)
	$(CC) $(CFLAGS) -o lavagna $(SERVER_SRCS)
 
utente: $(CLIENT_SRCS)
	$(CC) $(CFLAGS) -o utente $(CLIENT_SRCS)
 
clean:
	rm -f lavagna utente
 
