// header_builder.h
#ifndef HEADER_BUILDER_H
#define HEADER_BUILDER_H

#include "protocol/aliases.h"
#include "protocol/constant.h"
#include "../initiator/initiator.h"
#include <stdint.h>
#include <stddef.h>

#define IKE_HEADER_DIM sizeof(ike_header_t)

/*#######################################################
IKE Header Struct
#######################################################*/
typedef struct {
    uint64_t initiator_spi;   
    uint64_t responder_spi;  
    uint8_t next_payload; 
    uint8_t version;        
    uint8_t exchange_type; 
    uint8_t flags;        
    uint32_t message_id;  
    uint32_t length;     
} __attribute__((packed)) ike_header_t;

/*#######################################################
Pattern Builder
#######################################################*/
typedef struct {
    ike_header_t header;
} ike_header_builder_t;

ike_header_builder_t* create_builder();
void destroy_builder(ike_header_builder_t *builder);
void set_exchange_type(ike_header_builder_t *builder, ExchangeType exchange_type);
void set_length(ike_header_builder_t *builder, uint32_t length);
void set_flags(ike_header_builder_t *builder, const uint8_t flags[], size_t num_flags);
void set_message_id(ike_header_builder_t *builder, uint32_t message_id);
void set_initiator_spi(ike_header_builder_t *builder, uint64_t initiator_spi);
void set_responder_spi(ike_header_builder_t *builder, uint64_t responder_spi);
void set_next_payload(ike_header_builder_t *builder, NextPayload next_payload);
ike_header_t build_header(ike_header_builder_t *builder);

void create_header(ike_header_builder_t *builder, Initiator *initiator, ExchangeType exchange);
void parse_header(ike_header_t *header, uint8_t* buffer, size_t* buffer_len);

#endif // HEADER_BUILDER_H