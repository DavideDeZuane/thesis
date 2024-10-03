#include "header.h"
#include "protocol/aliases.h"
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <endian.h>

ike_header_builder_t* create_builder() {
    ike_header_builder_t *builder = (ike_header_builder_t*)malloc(sizeof(ike_header_builder_t));
    if (builder){ 
        memset(builder, 0, sizeof(ike_header_builder_t));
        builder->header.version |= 0x20; 
    }
    return builder;
}

void destroy_builder(ike_header_builder_t *builder) { free(builder); }
ike_header_t build_header(ike_header_builder_t *builder) { return builder->header; }

void set_next_payload(ike_header_builder_t *builder, NextPayload next_payload){ builder->header.next_payload = next_payload; }
void set_exchange_type(ike_header_builder_t *builder, ExchangeType exchange_type) { builder->header.exchange_type = exchange_type; }
void set_flags(ike_header_builder_t *builder, const uint8_t flags[], size_t num_flags) { for (size_t i = 0; i < num_flags; ++i) { builder->header.flags |= flags[i]; } }
void set_length(ike_header_builder_t *builder, uint32_t length) { builder->header.length = length; }
void set_message_id(ike_header_builder_t *builder, uint32_t message_id) { builder->header.message_id = message_id; }
void set_initiator_spi(ike_header_builder_t *builder, uint64_t initiator_spi) { builder->header.initiator_spi = initiator_spi; }
void set_responder_spi(ike_header_builder_t *builder, uint64_t responder_spi) { builder->header.responder_spi = responder_spi; }

/*##########################################################################################
Utility Varie
##########################################################################################*/
void create_header(ike_header_builder_t *builder, Initiator *initiator, ExchangeType exchange){
    switch(exchange){
        case EXCHANGE_IKE_SA_INIT: {
            set_initiator_spi(builder, initiator->spi);
            set_responder_spi(builder, SPI_NULL);
            set_next_payload(builder, NEXT_PAYLOAD_SA);
            set_exchange_type(builder, exchange);
            uint8_t flags[] = { FLAG_I };
            set_flags(builder, flags, sizeof(flags)/sizeof(flags[0]));
            set_message_id(builder, MID_NULL);
            set_length(builder, 72);
            break;
        };
        case EXCHANGE_IKE_AUTH: {
            break;
        };
        case EXCHANGE_CREATE_CHILD_SA: {
            break;
        };
        case EXCHANGE_INFORMATIONAL: {
            break;
        }
    }
}

void parse_header(ike_header_t *hdr, uint8_t* buffer, size_t* buffer_len){
    hdr->initiator_spi = be64toh(*(uint64_t*)&buffer[0]); 
    hdr->responder_spi = be64toh(*(uint64_t*)&buffer[8]);  
    hdr->next_payload = buffer[16];
    hdr->version = buffer[17];                  
    hdr->exchange_type = buffer[18];             
    hdr->flags = buffer[19];                      
    hdr->message_id = ntohl(*(uint32_t*)&buffer[20]);   
    hdr->length = ntohl(*(uint32_t*)&buffer[24]);  

    /*
        printf("SPI Initiator: 0x%016lx\n", hdr.initiator_spi);
    printf("SPI Responder: 0x%016lx\n", hdr.responder_spi);
    printf("Next Payload: %d\n", hdr.next_payload);
    printf("Version: %d\n", hdr.version);
    printf("Flags: %d\n", hdr.flags);
    printf("Message Type: %d\n", hdr.exchange_type);
    printf("Identifier: 0x%x\n", hdr.message_id);
    printf("Length: %d\n", hdr.length);
    */
}