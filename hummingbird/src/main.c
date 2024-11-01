#include "config/config.h"
#include "ike/header.h"
#include "ike/payload.h"
#include "ike/protocol/aliases.h"
#include "initiator/initiator.h"
#include "initiator/responder.h"
#include "ike/ike.h"
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <endian.h>
#include <arpa/inet.h>
#include <sched.h>
#include <time.h>
//librerie per dh
#include <openssl/evp.h>
#include <openssl/dh.h>
#include <openssl/err.h>
#include <openssl/provider.h>
#include <openssl/core_names.h>
#include <sys/socket.h>
#include <sys/types.h>

#define CONFIG_FILE "config.json"
#define BUFFER_SIZE 1024
#define PAYLOAD_SIZE 64 


typedef struct LookupEntry {
    uint8_t type;          // Tipo di payload
    uint8_t* data;        // Dati del payload
    uint16_t length;      // Lunghezza del payload
    struct LookupEntry* next; // Puntatore al prossimo elemento
} LookupEntry;

void handleErrors() {
    ERR_print_errors_fp(stderr);
    abort();
}


void print_buffer(uint8_t* buffer, size_t buffer_len){
    for (size_t i = 0; i < buffer_len ; i++) { printf("%02X ", buffer[i]); }
    printf("\n");
}

// Funzione per creare una nuova entry nella lookup table
LookupEntry* create_lookup_entry(uint8_t type, uint8_t* data, uint16_t length) {
    LookupEntry* entry = malloc(sizeof(LookupEntry));
    if (!entry) {
        perror("Failed to allocate memory for lookup entry");
        return NULL;
    }
    entry->type = type;
    entry->data = malloc(length);
    if (!entry->data) {
        perror("Failed to allocate memory for payload data");
        free(entry);
        return NULL;
    }
    memcpy(entry->data, data, length);
    entry->length = length;
    entry->next = NULL;
    return entry;
}

// Funzione per aggiungere una entry alla lookup table
void add_to_lookup(LookupEntry** head, uint8_t type, uint8_t* data, uint16_t length) {
    LookupEntry* new_entry = create_lookup_entry(type, data, length);
    new_entry->next = *head;
    *head = new_entry;
}

void print_lookup_table(LookupEntry* head) {
    LookupEntry* current = head;
    while (current) {
        printf("Payload Type: %u, Length: %u\n", current->type, current->length);
        current = current->next;
    }
}

LookupEntry* find_entry(LookupEntry* head, uint8_t type) {
    LookupEntry* current = head;
    while (current) {
        if (current->type == type) {
            return current; // Restituisce l'entry trovata
        }
        current = current->next; // Passa all'entry successiva
    }
    return NULL; // Restituisce NULL se non trova l'entry
}

// la chiamerei tipo funzione pacman
void parse_payload(uint8_t* buffer, size_t buffer_len, NextPayload current,  LookupEntry** lookup_table){
    if (current == 0) {
        printf("Fine dei payload: next payload è 0.\n");
        free(buffer);
        return;
    }
    uint8_t next_payload = buffer[0];
    uint16_t length = ntohs(*(uint16_t*)&buffer[2]);
    size_t payload_len = length - GENERIC_PAYLOAD_HEADER_DIM;
    uint8_t* payload_data = malloc(payload_len);
    memcpy(payload_data, &buffer[GENERIC_PAYLOAD_HEADER_DIM], payload_len);
    add_to_lookup(lookup_table, current, payload_data, payload_len);
    free(payload_data);

    if (next_payload != 0) {
        size_t next_buffer_size = buffer_len - length;
        parse_payload(buffer +length, next_buffer_size,next_payload, lookup_table);
    }
}

void print_bn(const char *label, const BIGNUM *bn) {
    char *bn_str = BN_bn2hex(bn);  // Convertiamo il BIGNUM in stringa esadecimale
    if (bn_str != NULL) {
        printf("%s: %s\n", label, bn_str);
        OPENSSL_free(bn_str);  // Dealloca la memoria
    } else {
        fprintf(stderr, "Errore nella conversione della BIGNUM.\n");
    }
}

void print_bytes(const void* obj, size_t size) {
    // Cast the pointer to an unsigned char, because we want to work with raw bytes
    const unsigned char* byte_ptr = (const unsigned char*)obj;

    printf("Bytes: ");
    for (size_t i = 0; i < size; ++i) {
        printf("%02X ", byte_ptr[i]);  // Stampa ogni byte come esadecimale
    }
    printf("\n");
}

unsigned char* derive_secret(EVP_PKEY *host_key, EVP_PKEY *peer_pub_key)
{
    OSSL_LIB_CTX *libctx = OSSL_LIB_CTX_new(); // Crea un nuovo contesto di libreria
    OSSL_PROVIDER_load(libctx, "legacy");
    OSSL_PROVIDER_load(libctx, "default");
    unsigned int pad = 1;
    OSSL_PARAM params[2];
    unsigned char *secret = NULL;
    size_t secret_len = 0;
    EVP_PKEY_CTX *dctx = EVP_PKEY_CTX_new_from_pkey(libctx, host_key, NULL);
    if (EVP_PKEY_derive_init(dctx) <= 0) {
        handleErrors();
    }
    
    if (EVP_PKEY_base_id(peer_pub_key) != EVP_PKEY_DH) {
        printf( "La chiave remota non è una chiave DH!\n");
    }   
   
    EVP_PKEY_derive_set_peer(dctx, peer_pub_key); 
    /* Get the size by passing NULL as the buffer */
    EVP_PKEY_derive(dctx, NULL, &secret_len);
    secret = OPENSSL_zalloc(secret_len);
    EVP_PKEY_derive(dctx, secret, &secret_len);
    return secret;
}


int main(void) {
    OPENSSL_init_crypto(0, NULL);
    struct sched_param sched_params;
    sched_params.sched_priority = 50;
    if (sched_setscheduler(0, SCHED_RR, &sched_params) == -1) { return EXIT_FAILURE;} 
    printf("Scheduler Utilizzato %d\n", sched_getscheduler(0));

    load("config.json");
    const Config* conf = get_config();
    Initiator initiator;
    initiator_init(&initiator, &conf->initiator);
    Responder responder;
    responder_init(&responder, &conf->responder);
    Proposal proposal = {0};
    create_proposal(&proposal, &conf->cipher_suite);
    KeyExchange ke = {0};
    ke.dh_group = htobe16(14);
    ke.reserved = 0;
    //aggiungere dh_group al metodo per la converisone a big endian
    //aggiungere il metodo che genera la chiave pubblica dh
    //per prima cosa occorre generare la chivave privata utilizzando il metodo EVP_PKEY-DHX
    OSSL_PARAM params[2];
    EVP_PKEY *pkey_local = NULL;
    EVP_PKEY_CTX *pctx = EVP_PKEY_CTX_new_from_name(NULL, "DH", NULL);
    //questa struttura permette di specificare dei parametri come il gruppo da utilizzare, la lunghezza della chiave
    params[0] = OSSL_PARAM_construct_utf8_string("group", "modp_2048", 0);
    params[1] = OSSL_PARAM_construct_end(); //questo indica la fine dei parametri
    //inizializziamo il contesto 
    EVP_PKEY_keygen_init(pctx);
    //si passano al contesto i parametri specificati
    EVP_PKEY_CTX_set_params(pctx, params);
    //questa genera effetticamente la cihave e la mette all'interno di pkey
    EVP_PKEY_generate(pctx, &pkey_local);
    BIGNUM *pub_key = NULL, *priv_key = NULL;
    // Otteniamo la chiave pubblica (pub_key) e chiave privata
    EVP_PKEY_get_bn_param(pkey_local, OSSL_PKEY_PARAM_PUB_KEY, &pub_key);
    EVP_PKEY_get_bn_param(pkey_local, OSSL_PKEY_PARAM_PRIV_KEY, &priv_key);
    size_t ke_data = BN_num_bytes(pub_key);
    // la conversione per la chiave pubblica è necessaria perchè dobbiamo inviarla come stream di byte
    BN_bn2bin(pub_key, ke.ke_data);
    uint8_t private[256] = {0};
    BN_bn2bin(priv_key, private);
    //la chiave privata non è necessario convertirla dato che possia
    EVP_PKEY_CTX_free(pctx);

    size_t nonce_len = 36;  
    uint8_t nonce[36];
    generate_nonce(nonce, nonce_len);

    PayloadHeader ni_header = {0};
    PayloadHeader ke_header = {0};
    PayloadHeader sa_header = {0};

    ike_header_builder_t *builder = create_builder();
    create_header(builder, &initiator, EXCHANGE_IKE_SA_INIT);
    ike_header_t header = build_header(builder);
    destroy_builder(builder);
    
    size_t packet_len = 0;
    uint8_t *packet = calloc(packet_len,  sizeof(uint8_t));
    // questa parte di codice popola il buffer con tutti i payload necessari per l' SA_INIT
    update_payload(PAYLOAD_TYPE_NONCE, &ni_header, &nonce, nonce_len);
    update_payload(PAYLOAD_TYPE_KE, &ke_header, &ke, sizeof(KeyExchange));
    update_payload(PAYLOAD_TYPE_SA, &sa_header, &proposal, sizeof(Proposal));
    generate_packet(&packet, &packet_len, &header);
    
    // prendere il tempo
    struct timespec start, end;
    double elapsed_time;

    size_t max_response_len = 500;
    uint8_t *response = calloc(max_response_len, sizeof(uint8_t));
    struct sockaddr_in cliaddr;
    socklen_t len = sizeof(cliaddr);
    clock_gettime(CLOCK_REALTIME, &start);
    sendto(initiator.socketfd, packet, packet_len, 0, (struct sockaddr*)&responder.node, sizeof(responder.node));

    /*
    printf("############################################################################################################################################\n");
    printf("PACCHETTO FINALE\n");
    printf("############################################################################################################################################\n");
    print_buffer(packet, packet_len);
    free(packet);
    */
    //mettere questa come variabile di dimensione massima

    //printf("In ascolto su porta %d...\n", htons(initiator.node.sin_port));
    // la recevfrom consente di specificare parametri extra inoltre come vaore di ritorno da la dimesione della risposta
    // dunque il buffer lo istanziamo a dimensione massimo poi quanto abbiamo ottenuto la dimensione effettiva che non è nota a priori
    // lo ridimensioniamo in modo tale da evitare spreco di memoria
    ssize_t n = recvfrom(initiator.socketfd, response, 1500, 0,  (struct sockaddr *)&cliaddr, &len );
    clock_gettime(CLOCK_REALTIME, &end);
    if (n < 0) {
        perror("Errore nella ricezione dei dati");
    }
    //fare il confronto tra i valori di ricezione del pacchetto con quelli specificati nel file di configurazione 
    // se non corrispondono scartiamo il pacchetto e facciamo report
    response = realloc(response, n);
    response[n] = '\0';  // Aggiungi terminatore di stringa
    size_t response_len = (size_t) n;
    printf("Dimensione della risposta %zd\n", n);
    printf("Messaggio ricevuto da %s:%d\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));

    // fare una funzione che si chiama parse response che fa il parsing dell'header e poi quello del payload
    ike_header_t hdr = {0};
    LookupEntry* lookup_table = NULL; 
    parse_header(&hdr, response, &response_len);
    parse_payload((response + IKE_HEADER_DIM), (response_len - IKE_HEADER_DIM), hdr.next_payload, &lookup_table);
    print_lookup_table(lookup_table);

    LookupEntry* pub_key_peer_bytes = find_entry(lookup_table, 34);
    //print_bytes(pub_key_peer_bytes->data, pub_key_peer_bytes->length); 
    EVP_PKEY* peer = EVP_PKEY_new(); //crea un contenitore vuoto per la chiave, questo può contenere sia chivi pubbliche che chiavi private che entrambe
    EVP_PKEY_copy_parameters(peer, pkey_local);
    // il +- 4 sono dovuti ai 4 byte di informazioni sul payload del ke

    unsigned char tmp[256] = {0};
    for(int i=0; i < 256; i++){
        tmp[i] = pub_key_peer_bytes->data[i+4];
    }

    if (EVP_PKEY_set1_encoded_public_key(peer, tmp, 256) <= 0)
        handleErrors();
    
    //EVP_PKEY* local = EVP_PKEY_new(); //crea un contenitore vuoto per la chiave, questo può contenere sia chivi pubbliche che chiavi private che entrambe
    //EVP_PKEY_set1_encoded_public_key(peer, private, 256);

    unsigned char* secret = derive_secret(pkey_local, peer);
    print_buffer(secret, 16);

    elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("Tempo trascorso: %f secondi\n", elapsed_time);
    /* 
    printf("############################################################################################################################################\n");
    printf("CHIAVI\n");
    printf("############################################################################################################################################\n");
    printf("Chiave Privata locale: \n");
    print_buffer(private, 256);
    printf("Chiave Pubblica locale: \n");
    print_buffer(ke.ke_data, 256);
    printf("Chiave Pubblica remota: \n");
    print_buffer(pub_key_peer_bytes->data+4, 256);
    */
    free(peer);
    free(lookup_table);

    return 0;
}