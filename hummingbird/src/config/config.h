#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

#define MAX_IP_LEN 45 
#define MAX_HOSTNAME_LEN 256
#define MAX_ALGORITHM_NAME_LEN 64
#define MAX_TYPE_LEN 5
#define NUM_ALGORITHMS 4

typedef struct {
    char ip[MAX_IP_LEN];
    char hostname[MAX_HOSTNAME_LEN];
    int port;
} Node;

typedef struct {
    uint16_t id;
    uint8_t type;
    char name[MAX_ALGORITHM_NAME_LEN];
} Algorithm;

typedef struct {
    Algorithm encryption;
    Algorithm integrity;
    Algorithm pseudorandom;
    Algorithm dh_group;
} Suite;

typedef struct {
    Node responder;
    Node initiator;
    Suite cipher_suite;
    int nat_traversal_port;
    int version;
} Config;

void load_config(const char *filename);
void load(const char *filename);
const Config* get_config();

#endif // CONFIG_H
