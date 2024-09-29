#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include "../config/config.h"
#include "../ike/ike.h"
#include "responder.h"
#include "initiator.h"
#include <stdlib.h>

uint64_t generate_spi() {
    uint8_t buffer[8];
    generate_nonce(buffer, 8);
    uint64_t spi_value = 0;
    for (int i = 0; i < sizeof(buffer); i++) spi_value = (spi_value << 8) | buffer[i];
    return spi_value;
}

int initiator_init(Initiator *initiator, const Node *conf){
    initiator->socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (initiator->socketfd < 0) {
        perror("socket creation failed");
        return -1;
    }

    memset(&initiator->node, 0, sizeof(initiator->node));
    initiator->node.sin_family = AF_INET;
    initiator->node.sin_port = htons(conf->port);
    (inet_pton(AF_INET, conf->ip, &initiator->node.sin_addr) <= 0) ? (perror("inet_pton failed"), exit(EXIT_FAILURE)) : (void)0;  //operatore ternario se la conversione avviene con successo viene eseguita la no-operation
    
    if (bind(initiator->socketfd, (struct sockaddr *)&initiator->node, sizeof(initiator->node)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    printf("Socket created!\n");
    initiator->spi = generate_spi();
    return 0;
}

//metodo per fare lo scambio ike sa init
//farei un metodo che specifica la richiesta che si vuole inviare e specifico anche il responder
void initiator_start(Initiator *initiator, Responder* responder){ 
    size_t packet_len = 0;
    uint8_t *packet = calloc(packet_len,  sizeof(uint8_t));
    //devo fare la funzione che mi genera il buffer che conterrà quel pachetto che poi devo invaire
    //ovvero che mi genera tutti i pyaload richiesti per lo scambio sa init quindi 

}

//popola il buffer che gli viene passato con quello che riceve
void receive_data(int sockfd, char *buffer, size_t buffer_size) {
    struct sockaddr_in server_addr;
    socklen_t addr_len = sizeof(server_addr);
    ssize_t recv_len = recvfrom(sockfd, buffer, buffer_size - 1, 0, (struct sockaddr *)&server_addr, &addr_len);
    if (recv_len < 0) {
        perror("recvfrom");
        exit(EXIT_FAILURE);
    }
    buffer[recv_len] = '\0'; // Aggiungi il terminatore di stringa
}

