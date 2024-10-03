#include <endian.h>
#include <stdint.h>
#include <string.h>
#include "payload.h"
#include "header.h"
#include "network.h"
#include <stdio.h>
#include "protocol/aliases.h"

#define SIZE_NULL 0

static PacketPayload init_payloads[] = {
    { NULL, NULL, SIZE_NULL, PAYLOAD_TYPE_NONCE },
    { NULL, NULL, SIZE_NULL, PAYLOAD_TYPE_KE },
    { NULL, NULL, SIZE_NULL, PAYLOAD_TYPE_SA }
};
static const size_t num_init_payloads = sizeof(init_payloads) / sizeof(init_payloads[0]);

PacketPayload* find_payload(int type) {
    for (size_t i = 0; i < num_init_payloads; i++) {
        if (init_payloads[i].type == type) {
            return &init_payloads[i];
        }
    }
    return NULL;  // Payload non trovato
}

// Funzione per aggiornare un payload esistente
void update_payload(int type, PayloadHeader *header, void *data, size_t data_len) {

    switch (type) {
        case PAYLOAD_TYPE_NONCE: {
            //spostare in qualche modo la generazione del nonce qua
            //eseguire il controllo per esempio se data_len corrisponde effetivamente alla dimensione del nonce
            create_generic_payload(header, NEXT_PAYLOAD_NONE, data_len);
            break;
        };
        case PAYLOAD_TYPE_KE: {
            create_generic_payload(header, NEXT_PAYLOAD_Ni, data_len);
            break;
        };
        case PAYLOAD_TYPE_SA: {
            create_generic_payload(header, NEXT_PAYLOAD_KE, data_len);
        };
    }

    PacketPayload *payload = find_payload(type);
    if (payload) {
        payload->header = header;
        payload->data = data;
        payload->data_len = data_len;
    } 
}

const PacketPayload* get_payloads(size_t *num_payloads){
    *num_payloads = num_init_payloads;
    return init_payloads;
}

//questa funzione la devo spostare nella funzione payload, la funzione network deve occuparsi solo della gestione della conversione
void generate_packet(unsigned char **packet, size_t *packet_len, ike_header_t *header) {
    // Ciclo su tutti i payloads definiti in init_payloads
    for (size_t i = 0; i < num_init_payloads; i++) {
        PacketPayload payload = init_payloads[i];
        if(payload.data == NULL) printf("Questo attributo è nullo %d\n", payload.type); 
        if(payload.data != NULL && payload.data_len > 0) {
            prepend_to_packet(packet, packet_len, payload.data, payload.data_len, payload.type);
        }
        if(payload.header != NULL){
            prepend_to_packet(packet, packet_len, payload.header, GENERIC_PAYLOAD_HEADER_DIM, GENERIC_PAYLOAD_HEADER);
        }
    }
    //una volta che ho aggiunto tutti i payload allora aggiorno la lunghezza del campo length nell'header e lo metto in testa al buffer
    header->length = IKE_HEADER_DIM + *packet_len;
    prepend_to_packet(packet, packet_len, header, IKE_HEADER_DIM, IKE_HEADER);
}

//qui mi serve la funzione che prende il buffer e me lo popola con tutte le informazioni per fare la richiesta
void generate_request(uint8_t **packet, size_t *packet_len, ExchangeType exchange){
    //quindi devo andare a popolare i vari container quindi definire header e payload
    //per ogni tipologia di payload che contiene vado a geneare payload e header quindi dovrei fare una funzione che mi cicla sui tipi 
}

void init_proposal(Proposal* proposal, const Suite* suite, ExchangeType exchange){
    switch(exchange){
        case EXCHANGE_IKE_SA_INIT:
            proposal->number = 1;
            proposal->spi_size = 0;
            proposal->protocol_id = PROTOCOL_ID_IKE;
            proposal->num_transforms = 4; //ovvero il numero di trasformazioni che vanno negoziate per IKE
        break;
        case EXCHANGE_IKE_AUTH:
        case EXCHANGE_CREATE_CHILD_SA:
        case EXCHANGE_INFORMATIONAL:
        break;
        
    }
}

void create_payload_header(PayloadHeader* header, NextPayload next_payload, uint16_t length){
    //fare controllo sui puntatori
    header->critical = header->reserved = 0;
    header->next_payload = next_payload;
    header->length = length;
}

void create_generic_payload(PayloadHeader* header, NextPayload next_payload, uint16_t length){
    header->next_payload = next_payload;
    header->critical = 0;
    header->length = GENERIC_PAYLOAD_HEADER_DIM + length;
}

void populate_transform(const Algorithm* alg, Proposal* proposal){
    
    Transform transform = {0};
    transform.type = alg->type;
    transform.id = alg->id;
    transform.length = sizeof(Transform);
    
    switch(alg->type){
        case TRANSFROM_TYPE_AUTH:{
            memcpy(&proposal->integ, &transform, transform.length);
            break;
        };
        case TRANSFORM_TYPE_ENCR:{
            transform.length += 4; //aggiungo la dimensione degli attributi
            memcpy(&proposal->encr, &transform, transform.length);
            break;
        };
        case TRANSFORM_TYPE_PRF:{
            memcpy(&proposal->prf, &transform, transform.length);
            break;
        };
        case TRANSFORM_TYPE_DHG:{
            memcpy(&proposal->dh_group, &transform, transform.length);
            break;
        };
    }
    proposal->length += transform.length;
}

//modificare e fare tutte trasformazioni con attribute tuttavia nel caso in cui non sono presenti trasformazioni lo rimuovo
void create_proposal(Proposal* proposal, const Suite* suite){
    init_proposal(proposal, suite, EXCHANGE_IKE_SA_INIT);
    populate_transform(&suite->integrity, proposal);
    populate_transform(&suite->encryption, proposal);
    populate_transform(&suite->dh_group, proposal);
    populate_transform(&suite->pseudorandom, proposal);
    proposal->length += 8; // alla proposal length non ho aggiunto la dimensione dei porcodio di campi che non ne fanno parte
    //per guardare bene il significato del campo attribute per questo caso fare riferimento all'RFC 7296
    proposal->attr.type = htobe16(0x800E);
    proposal->attr.value = htobe16(128);
}