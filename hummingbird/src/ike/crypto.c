#include <openssl/rand.h>
#include <string.h>

typedef struct {
    char *name;
    int id;
} AlgorithmIDMap;

AlgorithmIDMap id_map[] = {
    {"ENCR_AES_CBC",      12},
    {"ENCR_AES_CCM_8",    14},
    {"PRF_HMAC_SHA1",      2},
    {"AUTH_HMAC_SHA1_96",  2},
    {"AUTH_AES_XCBC_96",   5},
    {"DH_MODP2048",       14},
    {"DH_MODP1536",        5},
    {NULL, 0}
};

/**
 * @brief Ritorna l'ID IANA dell'algoritmo specificato.
 *
 * Utilizziamo la struttura chiave-valore definita per trovare l'id della primitiva crittografica da utilizzare 
 * dove il nome dell'algoritmo viene estratto dalla configurazione.
 *
 * @param name Il nome dell'algoritmo da cercare. 
 * @return L'ID dell'algoritmo corrispondente al nome fornito. Se l'algoritmo non è trovato, viene restituito -1.
 */
int find_algorithm_id(const char *name) {
    for (int i = 0; id_map[i].name != NULL; i++) {
        if (strcmp(id_map[i].name, name) == 0) {
            return id_map[i].id;
        }
    }
    return -1; 
}

/**
 * @brief Genera un nonce sicuro di lunghezza specificata.
 *
 * Questa funzione utilizza `RAND_bytes()` di OpenSSL per generare una sequenza di byte casuali.
 * I byte nulli vengono rigenerati per garantire che non ci siano byte nulli nel nonce.
 *
 * @param[out] nonce Buffer dove il nonce sarà salvato.
 * @param[in] nonce_len Lunghezza del nonce in byte.
 */
void generate_nonce(uint8_t *nonce, size_t nonce_len) {
    for (size_t i = 0; i < nonce_len; i++) {
            if (RAND_bytes(&nonce[i], 1) != 1) {
                fprintf(stderr, "Errore: impossibile generare nonce sicuro\n");
                exit(EXIT_FAILURE);
            }
    }
}

// aggiungere qua i metodi per DH
