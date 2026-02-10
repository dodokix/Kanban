CC = gcc
# Includiamo le directory degli header file
CFLAGS = -Wall -Wextra -g -Isrc

# Aggiungiamo protocol.c ai sorgenti del server
SERVER_SRCS = src/server/server.c src/server/net/net.c src/server/core/core.c src/shared/protocol/protocol.c
SERVER_OBJS = $(SERVER_SRCS:.c=.o)
SERVER_TARGET = server_test

# Aggiungiamo protocol.c e client/core/core.c ai sorgenti del client
CLIENT_SRCS = src/client/client.c src/client/net/net.c src/client/core/core.c src/shared/protocol/protocol.c
CLIENT_OBJS = $(CLIENT_SRCS:.c=.o)
CLIENT_TARGET = client_test

all: $(SERVER_TARGET) $(CLIENT_TARGET)

$(SERVER_TARGET): $(SERVER_OBJS)
	$(CC) $(CFLAGS) -o $(SERVER_TARGET) $(SERVER_OBJS)

$(CLIENT_TARGET): $(CLIENT_OBJS)
	$(CC) $(CFLAGS) -o $(CLIENT_TARGET) $(CLIENT_OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(SERVER_OBJS) $(SERVER_TARGET) $(CLIENT_OBJS) $(CLIENT_TARGET)

run_server: $(SERVER_TARGET)
	./$(SERVER_TARGET)

# Esegui con: make run_client PORT=5679
run_client: $(CLIENT_TARGET)
	./$(CLIENT_TARGET) $(PORT)

.PHONY: all clean run_server run_client