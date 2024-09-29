#include <cjson/cJSON.h>
#include "config.h"
#include "../ike/ike.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

static Config config;

char* concatenate_algorithms(Suite* suite) {
    const char* algorithms[NUM_ALGORITHMS] = {
        suite->encryption.name,
        suite->pseudorandom.name,
        suite->integrity.name,
        suite->dh_group.name
    };
    size_t total_length = 0;
    for (int i = 0; i < NUM_ALGORITHMS; i++) { total_length += strlen(algorithms[i]); }
    total_length += (NUM_ALGORITHMS - 1) * 1; // 3 delimitatori
    total_length += 1; // Terminatore null

    char *result = (char *)malloc(total_length + 1);
    if (result == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    result[0] = '\0';
    for (int i = 0; i < NUM_ALGORITHMS; i++) {
        strcat(result, algorithms[i]);
        if (i < NUM_ALGORITHMS - 1) { 
            strcat(result, "-");
        }
    }
    return result;
}

char* read_file(const char* filename){
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Could not open config file");
        exit(EXIT_FAILURE);
    }
    printf("Loading configuration file: \033[1;32m%s\033[0m \n", filename);
    fseek(file, 0, SEEK_END); //questa funzione sposta il puntatore alla fine del file
    long length = ftell(file); //questa funzione restituisce la posizione del puntatore in byte, dato che si trova alla fine del file troviamo la dimensione del file in byte
    fseek(file, 0, SEEK_SET); //sposta il puntatore all'inizio del file
    char* data = malloc(length + 1); //allochiamo un buffer dinamico di lunghezza lenght +1 per contenere il contenuto del file
    fread(data, 1, length, file); //leggiamo tutto il file
    fclose(file);
    data[length] = '\0'; //aggiungiamo il carattere di fine stringa per poter trattare il buffer come una stringa
    return data;
}

void parse_node(const cJSON* prop, Node *node){
    if(cJSON_IsObject(prop)){
        const cJSON *ip = cJSON_GetObjectItemCaseSensitive(prop, "ip");
        const cJSON *port = cJSON_GetObjectItemCaseSensitive(prop, "port");
        const cJSON *hostname = cJSON_GetObjectItemCaseSensitive(prop, "hostname");
        if (cJSON_IsString(ip) && ip->valuestring != NULL) strncpy(node->ip, ip->valuestring, INET_ADDRSTRLEN); //si assicura che la copia non superi la dimensione massima definita da INET_ADDRSTRLEN
        if (cJSON_IsNumber(port)) node->port = port->valueint;
        if (cJSON_IsString(hostname) && hostname->valuestring != NULL) strncpy(node->hostname, hostname->valuestring, MAX_HOSTNAME_LEN); //si assicura che la copia non superi la dimensione massima definita da INET_ADDRSTRLEN
    }
    //nel caso in cui alcune proprietà non passano i test mandare un messaggio di errore che la configurazione è sbagliata
}

void concat_type_algorithm(const char *type, const char *algorithm, char *output) {
    snprintf(output, MAX_ALGORITHM_NAME_LEN + MAX_TYPE_LEN, "%s_%s", type, algorithm);
}


void parse_alg(const cJSON* prop, Algorithm* alg, TransformType type){
    char id[MAX_ALGORITHM_NAME_LEN + MAX_TYPE_LEN];
    strncpy(alg->name, prop->valuestring, MAX_ALGORITHM_NAME_LEN);
    char tmp[MAX_TYPE_LEN];
    switch(type){
        case TRANSFORM_TYPE_ENCR: {
            alg->type = TRANSFORM_TYPE_ENCR;
            strncpy(tmp, "ENCR", MAX_TYPE_LEN-1);
            break;
        };
        case TRANSFORM_TYPE_PRF: {
            alg->type = TRANSFORM_TYPE_PRF;
            strncpy(tmp, "PRF", MAX_TYPE_LEN-1);
            break;
        };
        case TRANSFROM_TYPE_AUTH: {
            alg->type = TRANSFROM_TYPE_AUTH;
            strncpy(tmp, "AUTH", MAX_TYPE_LEN-1);
            break;
        };
        case TRANSFORM_TYPE_DHG: {
            alg->type = TRANSFORM_TYPE_DHG;
            strncpy(tmp, "DH", MAX_TYPE_LEN-1);
            break;
        };
        case TRANSFORM_TYPE_ESN: {
            break;
        };
    }
    concat_type_algorithm(tmp, alg->name, id);
    alg->id = find_algorithm_id(id); //la funzione nel caso di errore restituisce -1 e si verifica il overflow e il valore diventa 255
    if(alg->id == 255) printf("Algoritmo non supportato");

}

void parse_cipher_suite(const cJSON* conf, Suite* suite){
    if(!cJSON_IsObject(conf)) {
        printf("Controllare se il nome della proprietà è corretto\n");
        return;
    }
    const cJSON *encryption = cJSON_GetObjectItemCaseSensitive(conf, "Encryption"); 
    const cJSON *pseudorandom = cJSON_GetObjectItemCaseSensitive(conf, "Pseudorandom"); 
    const cJSON *integrity = cJSON_GetObjectItemCaseSensitive(conf, "Integrity"); 
    const cJSON *dh_group = cJSON_GetObjectItemCaseSensitive(conf, "DH Group"); 
    parse_alg(encryption, &suite->encryption, TRANSFORM_TYPE_ENCR);
    parse_alg(pseudorandom, &suite->pseudorandom, TRANSFORM_TYPE_PRF);
    parse_alg(integrity, &suite->integrity, TRANSFROM_TYPE_AUTH);
    parse_alg(dh_group, &suite->dh_group, TRANSFORM_TYPE_DHG);
    printf("Chiper suite loaded: \033[1;33m%s\n\033[0m", concatenate_algorithms(suite));
}

void parse_file(const char* json, Config* conf){
    cJSON *root = cJSON_Parse(json);
    if (!root) fprintf(stderr, "Error parsing JSON\n");
    /*####################################################################################
    Parsing Json Object
    ####################################################################################*/
    const cJSON *responder_cfg = cJSON_GetObjectItemCaseSensitive(root, "responder");
    const cJSON *initiator_cfg = cJSON_GetObjectItemCaseSensitive(root, "initiator"); 
    const cJSON *cipher_suite_cfg = cJSON_GetObjectItemCaseSensitive(root, "proposal"); 
    parse_node(responder_cfg, &conf->responder);
    parse_node(initiator_cfg, &conf->initiator);
    parse_cipher_suite(cipher_suite_cfg, &conf->cipher_suite);
    /*####################################################################################
    Parsing Proprietà globali 
    ####################################################################################*/
    const cJSON *nat_traversal_port = cJSON_GetObjectItemCaseSensitive(root, "nat_traversal_port");
    const cJSON *ike_version = cJSON_GetObjectItemCaseSensitive(root, "version");
    if (cJSON_IsNumber(nat_traversal_port)) conf->nat_traversal_port = nat_traversal_port->valueint;
    if (cJSON_IsNumber(ike_version)) conf->version = ike_version->valueint;
    cJSON_Delete(root);
}

void load(const char* filename){
    char* data = read_file(filename);
    parse_file(data, &config);
    free(data);
    printf("Loading Complete!\n");
}

const Config* get_config(void) { return &config; }
