#include "network.h"
#include "header.h"
#include "payload.h"
#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


/**
 * @brief Prepend data to an existing packet.
 *
 * Questa funzione premette dei dati a un pacchetto esistente, allocando dinamicamente
 * un nuovo buffer per il pacchetto risultante. I dati da premettere vengono forniti
 * tramite il parametro `data`, e possono essere di qualsiasi tipo specificato dal tipo
 * `data_struct_type_t`. La lunghezza del pacchetto e il puntatore al buffer del pacchetto
 * vengono aggiornati in base ai nuovi dati.
 *
 * @param[in,out] packet       Puntatore al buffer del pacchetto. Viene aggiornato per contenere il nuovo buffer con i dati pre-posti.
 * @param[in,out] packet_len   Puntatore alla lunghezza del pacchetto. Viene aggiornato per riflettere la nuova dimensione del pacchetto.
 * @param[in]     data         Puntatore ai dati da premettere al pacchetto.
 * @param[in]     data_len     Lunghezza dei dati da premettere in byte.
 * @param[in]     type         Tipo dei dati che vengono preposti al pacchetto, indicato dal tipo `data_struct_type_t`.
 *
*/
void prepend_to_packet(uint8_t **packet, size_t *packet_len, void *data, int data_len, PayloadType type){
    //rialloco il buffer per fare in modo che i dati che voglio aggiungere c'entrino
    *packet = (uint8_t *)realloc(*packet, *packet_len + data_len);
    memmove(*packet + data_len, *packet, *packet_len);
    convert_to_big_endian(data, type);
    memcpy(*packet, data, data_len);
    convert_to_little_endian(data, type);
    *packet_len += data_len;
    printf("Dati aggiunti al pacchetto: ");

    uint8_t *buf = (uint8_t *) data;
    for (size_t i = 0; i < data_len; i++) {
        printf("0x%02x ", buf[i]);
    }
    printf("\n");
}

void convert_transform(Transform* transform) {
    convert_to_big_endian(transform, TRANSFORM);
}


/**
 * @brief Esegue la conversione dei dati in formato big endian
 *
 * Questa funzione permette di convertire una struttura dati nel formato big endian, dato che nel 
 * caso di processo x86 questi adottano un'architettura little endian. Mentre lo stack TCP/IP 
 * funziona solamente con quella. 
 * Il tipo serve per definire quali sono i campi delle varie strutture che vanno modificati per essere convertiti
 *
 * @param[in,out] data  E' un tipo generico che punta alla struttura dati da convertire 
 * @param[in]     type  Il tipo di dato che si vuole andare a convertire
 *
*/
void convert_to_big_endian(void *data, PayloadType type) {
    field_descriptor_t *fields = NULL;
    size_t fields_num = 0;
    //switch case per fare la conversione dei vari campi
    switch(type){
        case IKE_HEADER: {
            field_descriptor_t tmp[] = {
                { offsetof(ike_header_t, initiator_spi), FIELD_UINT64 },
                { offsetof(ike_header_t, responder_spi), FIELD_UINT64 },
                { offsetof(ike_header_t, message_id), FIELD_UINT32 },
                { offsetof(ike_header_t, length), FIELD_UINT32 }
            };            
            fields = (field_descriptor_t *) tmp;
            fields_num = sizeof(tmp) / sizeof(tmp[0]);
            break;
        };
        case GENERIC_PAYLOAD_HEADER:
         {
            field_descriptor_t tmp[] = { { offsetof(PayloadHeader, length), FIELD_UINT16 } };
            fields = (field_descriptor_t *) tmp;
            fields_num = sizeof(tmp) / sizeof(tmp[0]);
            break;
        };
        case TRANSFORM: {
            field_descriptor_t tmp[] = {
                { offsetof(Transform, id), FIELD_UINT16 },
                { offsetof(Transform, length), FIELD_UINT16 },
            };
            fields = (field_descriptor_t *) tmp;
            fields_num = sizeof(tmp) / sizeof(tmp[0]);
            break;
        };
        case PAYLOAD_TYPE_SA: {
            field_descriptor_t tmp[] = { { offsetof(Proposal, length), FIELD_UINT16 } };
            Proposal *prop = (Proposal *) data;
            convert_transform(&prop->integ);
            convert_transform(&prop->dh_group);
            convert_transform(&prop->prf);
            convert_transform(&prop->encr);
            fields = (field_descriptor_t *) tmp;
            fields_num = sizeof(tmp) / sizeof(tmp[0]);
            break;
        };
        case PAYLOAD_TYPE_NONE:
        case PAYLOAD_TYPE_NONCE:
        case PAYLOAD_TYPE_KE:
            return;
    }

    //funzione che esegue la conversione
    for (size_t i = 0; i < fields_num; i++) {
        void *field_ptr = (uint8_t *)data + fields[i].offset;

        switch (fields[i].type) {
            case FIELD_UINT16: {
                uint16_t *value = (uint16_t *)field_ptr;
                *value = htobe16(*value);  // Conversione in big-endian per uint16_t
                break;
            }
            case FIELD_UINT32: {
                uint32_t *value = (uint32_t *)field_ptr;
                *value = htobe32(*value);  // Conversione in big-endian per uint32_t
                break;
            }
            case FIELD_UINT64: {
                uint64_t *value = (uint64_t *)field_ptr;
                *value = htobe64(*value);  // Conversione in big-endian per uint64_t
                break;
            }
        }
    }
}