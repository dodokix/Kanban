CC = gcc
# -Isrc serve per far trovare net/net.h al compilatore
CFLAGS = -Wall -Wextra -g -Isrc

# Percorsi relativi alla cartella dove si trova il Makefile
SERVER_SRCS = src/server/server.c src/server/net/net.c 
SERVER_OBJS = $(SERVER_SRCS:.c=.o)
SERVER_TARGET = server_test

all: $(SERVER_TARGET)

server: $(SERVER_TARGET)

$(SERVER_TARGET): $(SERVER_OBJS)
	$(CC) $(CFLAGS) -o $(SERVER_TARGET) $(SERVER_OBJS)

# Questa regola dice: per creare un .o, cerca il .c nella stessa posizione
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(SERVER_OBJS) $(SERVER_TARGET)

run_server: $(SERVER_TARGET)
	./$(SERVER_TARGET)

.PHONY: all clean run_server