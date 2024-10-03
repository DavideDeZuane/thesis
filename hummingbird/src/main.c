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
//librerie per dh
#include <openssl/evp.h>
#include <openssl/dh.h>
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


void print_buffer(uint8_t* buffer, size_t buffer_len){
    for (size_t i = 0; i < buffer_len ; i++) { printf("%02X ", buffer[i]); }
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
        return;
    }
    uint8_t next_payload = buffer[0];
    uint16_t length = ntohs(*(uint16_t*)&buffer[2]);
    printf("Payload attuale: %d \n\n", current);
    size_t payload_len = length - GENERIC_PAYLOAD_HEADER_DIM;
    uint8_t* payload_data = malloc(payload_len);
    memcpy(payload_data, &buffer[GENERIC_PAYLOAD_HEADER_DIM], payload_len);
    add_to_lookup(lookup_table, current, payload_data, payload_len);
    print_buffer(buffer, length);
    free(payload_data);


    if (next_payload != 0) {
        size_t next_buffer_size = buffer_len - length;
        parse_payload(buffer +length, next_buffer_size,next_payload, lookup_table);
    }
    //parse_payload(buffer + length, buffer_len-length, next_payload);

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

int main(void) {
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
    EVP_PKEY *pkey = NULL;
    EVP_PKEY_CTX *pctx = EVP_PKEY_CTX_new_from_name(NULL, "DH", NULL);
    //questa struttura permette di specificare dei parametri come il gruppo da utilizzare, la lunghezza della chiave
    params[0] = OSSL_PARAM_construct_utf8_string("group", "modp_2048", 0);
    params[1] = OSSL_PARAM_construct_end(); //questo indica la fine dei parametri
    //inizializziamo il contesto 
    EVP_PKEY_keygen_init(pctx);
    //si passano al contesto i parametri specificati
    EVP_PKEY_CTX_set_params(pctx, params);
    //questa genera effetticamente la cihave e la mette all'interno di pkey
    EVP_PKEY_generate(pctx, &pkey);
    BIGNUM *pub_key = NULL, *priv_key = NULL;
    // Otteniamo la chiave pubblica (pub_key) dal pkey
    EVP_PKEY_get_bn_param(pkey, OSSL_PKEY_PARAM_PUB_KEY, &pub_key);
    size_t ke_data = BN_num_bytes(pub_key);
    BN_bn2bin(pub_key, ke.ke_data);


    size_t nonce_len = 240;  
    uint8_t nonce[240];

    // Genera il nonce casuale
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
    update_payload(PAYLOAD_TYPE_NONCE, &ni_header, &nonce, 240);
    update_payload(PAYLOAD_TYPE_KE, &ke_header, &ke, sizeof(KeyExchange));
    update_payload(PAYLOAD_TYPE_SA, &sa_header, &proposal, sizeof(Proposal));
    generate_packet(&packet, &packet_len, &header);


    sendto(initiator.socketfd, packet, packet_len, 0, (struct sockaddr*)&responder.node, sizeof(responder.node));
    
    printf("############################################################################################################################################\n");
    printf("PACCHETTO FINALE\n");
    printf("############################################################################################################################################\n");
    print_buffer(packet, packet_len);
    //for (size_t i = 0; i < packet_len ; i++) { printf("%02X ", packet[i]); }

    size_t response_len = 500;
    uint8_t *response = calloc(response_len, sizeof(uint8_t));
    struct sockaddr_in cliaddr;
    socklen_t len = sizeof(cliaddr);


    printf("In ascolto su porta %d...\n", htons(initiator.node.sin_port));

    // la recevfrom consente di specificare parametri extra inoltre come vaore di ritorno da la dimesione della risposta
    // dunque il buffer lo istanziamo a dimensione massimo poi quanto abbiamo ottenuto la dimensione effettiva che non è nota a priori
    // lo ridimensioniamo in modo tale da evitare spreco di memoria
    ssize_t n = recvfrom(initiator.socketfd, response, 1500, 0,  (struct sockaddr *)&cliaddr, &len );
    if (n < 0) {
        perror("Errore nella ricezione dei dati");
    }
    //fare il confronto tra i valori di ricezione del pacchetto con quelli specificati nel file di configurazione 
    // se non corrispondono scartiamo il pacchetto e facciamo report

    response = realloc(response, n);
    printf("Dimensione della risposta %zd\n", n);
    response[n] = '\0';  // Aggiungi terminatore di stringa
    printf("Messaggio ricevuto da %s:%d\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));

    print_buffer(response, n);

    //trasformarlo in una funzione parse_response_header tanto i campi sono fissi non c'è altro modo
    ike_header_t hdr = {0};
    hdr.initiator_spi = be64toh(*(uint64_t*)&response[0]);  // SPI Initiator (8 byte)
    hdr.responder_spi = be64toh(*(uint64_t*)&response[8]);  // SPI Responder (8 byte)
    hdr.next_payload = response[16];
    hdr.version = response[17];                             // Versione (1 byte)
    hdr.exchange_type = response[18];                        // Tipo di messaggio (1 byte)
    hdr.flags = response[19];                        // Tipo di messaggio (1 byte)
    hdr.message_id = ntohl(*(uint32_t*)&response[20]);   
    hdr.length = ntohl(*(uint32_t*)&response[24]);         // Lunghezza del messaggio (2 byte)

    // Fare il file di log per vedere le varie cose che vengono effettuate 
    printf("SPI Initiator: 0x%016lx\n", hdr.initiator_spi);
    printf("SPI Responder: 0x%016lx\n", hdr.responder_spi);
    printf("Next Payload: %d\n", hdr.next_payload);
    printf("Version: %d\n", hdr.version);
    printf("Flags: %d\n", hdr.flags);
    printf("Message Type: %d\n", hdr.exchange_type);
    printf("Identifier: 0x%x\n", hdr.message_id);
    printf("Length: %d\n", hdr.length);

    //a questo punto a partire dalla dimensione del buffer devo iniziare a fare il parsing dei next payload per determinare quanto deve essere la 
    //lettura dei vari payload
    // fare una funzione che prende il buffer ed estra il payload
    size_t payload_len = response_len - IKE_HEADER_DIM;
    uint8_t* payload = malloc(payload_len);
    memcpy(payload, response + IKE_HEADER_DIM, payload_len);
    free(response);
    print_buffer(payload, payload_len);

    // a questo punto posso andare a fare il parsing dei next payload quindi

    PayloadHeader first = {0};

    first.next_payload = payload[0];
    first.length = ntohs(*(uint16_t*)&payload[2]); //facciamo un cast del puntatore al 3 byte, invece che essere un puntatore ad un campo a 8 bit diventa un puntatore ad un campo di 16 bit, quindi lo consideriamo come un valore unico

    printf("Generic Payload header: %d \n", first.next_payload);
    printf("Generic Payload length: %u \n", first.length);

    // il parsing della proposal per il momento non mi interessa poichè se mi ha risposto vuol dire che gli vabene
    // andiamo a prendere direttamente il materiale per il key exchange
    // ad ogni payload che parso ridimensiono il buffer 
    //invece che allocare ogni volta un buffer di dimensione minore fare un buffer pool
    // oppure andare ad incrementare il buffer sulla base del campo lunghezza contenuto nel next payload
    LookupEntry* lookup_table = NULL; // Inizializza la lookup table
    parse_payload(payload, payload_len, hdr.next_payload, &lookup_table);


    // Stampa la lookup table
    print_lookup_table(lookup_table);

    LookupEntry* found_payload = find_entry(lookup_table, 34);
    if (found_payload) {
        printf("Payload Type: %u, Length: %u, Data: ", found_payload->type, found_payload->length);
        print_buffer(found_payload->data, found_payload->length);
    } else {
        printf("Payload di tipo non trovato.\n");
    }

   
    return 0;
}