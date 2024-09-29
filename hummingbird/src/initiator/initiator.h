#ifndef INITIATOR_H
#define INITIATOR_H

#include "../config/config.h"
#include <stddef.h>
#include <netinet/in.h> // Per struct sockaddr_in

typedef struct {
    int socketfd;
    uint64_t spi;    
    struct sockaddr_in node;
} Initiator;

int initiator_init(Initiator *initiator, const Node *conf);
void receive_data(int sockfd, char *buffer, size_t buffer_size);

#endif