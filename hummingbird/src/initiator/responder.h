#ifndef RESPONDER_H
#define RESPONDER_H

#include <netinet/in.h>
#include "../config/config.h"

// Definizione della struct per il Responder
typedef struct {
    struct sockaddr_in node;   // Informazioni di rete del Responder
    uint64_t spi;
} Responder;

int responder_init(Responder* responder, const Node* conf);

#endif