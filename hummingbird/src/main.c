#include "config/config.h"
#include "initiator/initiator.h"
#include "initiator/responder.h"
#include "ike/ike.h"
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

#define CONFIG_FILE "config.json"
#define BUFFER_SIZE 1024
#define PAYLOAD_SIZE 64 

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
    for (size_t i = 0; i < packet_len ; i++) { printf("%02X ", packet[i]); }
   
    return 0;
}