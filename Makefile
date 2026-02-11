CC = gcc
CFLAGS = -Wall -Wextra -g -Isrc

SERVER_SRCS = src/server/server.c src/server/net/net.c src/server/core/core.c src/shared/protocol/protocol.c
CLIENT_SRCS = src/client/client.c src/client/net/net.c src/client/core/core.c src/shared/protocol/protocol.c
SHARED_SRCS = src/shared/protocol/protocol.c

SERVER_OBJS = $(SERVER_SRCS:.c=.o)
CLIENT_OBJS = $(CLIENT_SRCS:.c=.o)

SERVER_TARGET = server_test
CLIENT_TARGET = client_test

all: $(SERVER_TARGET) $(CLIENT_TARGET)

server: $(SERVER_TARGET)

$(SERVER_TARGET): $(SERVER_OBJS)
	$(CC) $(CFLAGS) -o $(SERVER_TARGET) $(SERVER_OBJS)

client: $(CLIENT_TARGET)

$(CLIENT_TARGET): $(CLIENT_OBJS)
	$(CC) $(CFLAGS) -o $(CLIENT_TARGET) $(CLIENT_OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(SERVER_OBJS) $(SERVER_TARGET) $(CLIENT_OBJS) $(CLIENT_TARGET)
	find . -name "*.o" -type f -delete

.PHONY: all clean run_server run_client