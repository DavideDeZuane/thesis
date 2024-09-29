#ifndef CRYPTO_H
#define CRYPTO_H

#include <stdint.h>
#include <stddef.h>

#define NONCE_MAX_SIZE 32

/**
 * @brief Popola il buffer con un nonce della lunghezza specificata.
 *
 * @param[out] nonce la funzione modifica il buffer e mette al suo interno il nonce generato.
 * @param[in] nonce_len Lunghezza del nonce in byte.
 */
void generate_nonce(uint8_t *nonce, size_t nonce_len);

/**
 * @brief Ritorna l'ID IANA dell'algoritmo specificato.
 *
 * @param name Il nome dell'algoritmo da cercare. Deve essere una stringa terminata da null.
 * @return L'ID dell'algoritmo corrispondente al nome fornito. Se l'algoritmo non è trovato, viene restituito -1.
 */
int find_algorithm_id(const char *name);


#endif