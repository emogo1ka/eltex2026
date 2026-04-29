#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>

#define MAX_PAYLOAD 1024
#define SHUTDOWN_MSG "SHUTDOWN_DISCONNECT"

// Pseudo header for UDP checksum
struct pseudo_header {
    uint32_t source_address;
    uint32_t dest_address;
    uint8_t placeholder;
    uint8_t protocol;
    uint16_t udp_length;
};

// Client identification structure
typedef struct {
    uint32_t ip;
    uint16_t port;
} client_id_t;

// Node for message counter linked list
typedef struct client_node {
    client_id_t id;
    int count;
    struct client_node *next;
} client_node_t;

// Utility functions
unsigned short calculate_checksum(unsigned short *ptr, int nbytes);
client_node_t* find_or_create_client(client_node_t **head, client_id_t id);
void remove_client(client_node_t **head, client_id_t id);

#endif
