#ifndef PAYLOAD_BUILDER_H
#define PAYLOAD_BUILDER_H

#include <stdint.h>
#include <endian.h>
#include <stdint.h>
#include <stddef.h>
#include "../config/config.h"
#include "protocol/aliases.h"
#include "header.h"

#define MAX_KEY_EXCHANGE_DATA_LEN 256
#define GENERIC_PAYLOAD_HEADER_DIM sizeof(PayloadHeader)

typedef enum {
    PAYLOAD_TYPE_NONE,
    PAYLOAD_TYPE_NONCE,
    PAYLOAD_TYPE_KE,
    PAYLOAD_TYPE_SA,
    GENERIC_PAYLOAD_HEADER,
    IKE_HEADER,
    TRANSFORM,
} PayloadType;

typedef struct {
    uint8_t  next_payload;  
    uint8_t  critical :1;  
    uint8_t  reserved :7;
    uint16_t length;        
} __attribute__((packed)) PayloadHeader;

typedef struct {
    PayloadHeader *header;
    void *data;          // Puntatore ai dati da aggiungere al pacchetto
    size_t data_len;     // Lunghezza dei dati
    int type;            // Tipo di payload
} PacketPayload;

typedef struct {
    uint16_t type;
    uint16_t value;
} __attribute__((packed)) Attribute;

typedef struct {
    uint8_t more;
    uint8_t reserved_1;
    uint16_t length;
    uint8_t type;
    uint8_t reserved_2;
    uint16_t id;
} __attribute__((packed)) Transform;

typedef struct {
    uint8_t last_or_more;
    uint8_t reserved;
    uint16_t length; //dato che il payload può avere dimensioni variabili a seconda di quante proposte facciamo
    uint8_t number; //identifica la proposta che viene fatta, va inizializzata una e le proposte successive devono incrementare il numero 
    uint8_t protocol_id;
    uint8_t spi_size;              // SPI Size (1 byte)
    uint8_t num_transforms;        // Numero di trasformazioni (1 byte)
    //uint8_t *spi;    
    Transform encr;
    Attribute attr;
    Transform integ;
    Transform dh_group;
    Transform prf;
} __attribute__((packed)) Proposal;

typedef struct {
    uint16_t dh_group;
    uint16_t reserved;
    uint8_t ke_data[MAX_KEY_EXCHANGE_DATA_LEN]; //contiene la chiave pubblica generata
} __attribute__((packed)) KeyExchange;

void create_proposal(Proposal *proposal, const Suite* suite);
void print_proposal(Proposal* proposal);
void create_generic_payload(PayloadHeader* header, NextPayload next_payload, uint16_t length);
void generate_packet(unsigned char **packet, size_t *packet_len, ike_header_t *header);

const PacketPayload* get_payloads(size_t *num_payloads);
void update_payload(int type, PayloadHeader *header, void *data, size_t data_len);

#endif