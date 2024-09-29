#ifndef NETWORK_H 
#define NETWORK_H

#include "header.h"
#include "payload.h"
#include <stddef.h>
#include <stdint.h>

typedef enum {
    FIELD_UINT16,
    FIELD_UINT32,
    FIELD_UINT64
} field_type_t;

typedef struct {
    size_t offset;
    field_type_t type;
} field_descriptor_t;

/**
 * @brief Questo enum serve per definire come deve avvenire la conversione big endian della struct
 
 * Specifichiamo il tipo della struttura dati che vogliamo convertire nello switch.
 * In ogni case per ogni tipo di struct definiamo quelli che sono i campi che devono essere convertiti
 */

/**
 * @brief Esegue la conversione big endian della struttura specificata e viceversa
 *
 * @param[out] data Struttura dati di cui si vuole convertire la rappresentazione
 * @param[in] type Tipo della struttura che si vuole convertire
 */
void convert_to_big_endian(void *data, PayloadType type);
#define convert_to_little_endian convert_to_big_endian

/**
 * @brief Mette in testa al buffer il contenuto specificato 
 *
 * @param[out] packet Buffer a cui si vuole aggiungere in testa dei dati
 * @param[out] packet_len Lunghezza del buffer
 * @param[in] data Struttura dati che si vuole aggiungere in testa
 * @param[in] data_len Lunghezza della struttura
 * @param[in] type Tipo della struttura che si vuole aggiungere
 *
*/
void prepend_to_packet(uint8_t **packet, size_t *packet_len, void *data, int data_len, PayloadType type);


#endif