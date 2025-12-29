#ifndef CLIENT_NET_H
#define CLIENT_NET_H

#include "../../shared/client_constants.h"

extern int port;

int configure_port();
void close_port();
