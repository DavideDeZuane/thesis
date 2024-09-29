#include "responder.h"
#include "../config/config.h"
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h> 


int responder_init(Responder* responder, const Node* conf){
    memset(&responder->node, 0, sizeof(responder));
    responder->node.sin_port = htons(conf->port);
    responder->node.sin_family = AF_INET;
    (inet_pton(AF_INET, conf->ip, &responder->node.sin_addr) <= 0) ? ( printf("errore")) : (void)0;  //operatore ternario se la conversione avviene con successo viene eseguita la no-operation
    return 0;
}