#include "udp_raw.h"

// Standard Internet Checksum implementation (RFC 1071)
unsigned short calculate_checksum(unsigned short *ptr, int nbytes) {
    long sum = 0;
    unsigned short oddbyte;
    short answer;

    while (nbytes > 1) {
        sum += *ptr++;
        nbytes -= 2;
    }
    if (nbytes == 1) {
        oddbyte = 0;
        *((unsigned char *)&oddbyte) = *(unsigned char *)ptr;
        sum += oddbyte;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum = sum + (sum >> 16);
    answer = (short)~sum;

    return answer;
}

client_node_t* find_or_create_client(client_node_t **head, client_id_t id) {
    client_node_t *curr = *head;
    while (curr) {
        if (curr->id.ip == id.ip && curr->id.port == id.port) {
            return curr;
        }
        curr = curr->next;
    }
    
    // Create new node if not found
    client_node_t *new_node = malloc(sizeof(client_node_t));
    if (!new_node) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    new_node->id = id;
    new_node->count = 0;
    new_node->next = *head;
    *head = new_node;
    return new_node;
}

void remove_client(client_node_t **head, client_id_t id) {
    client_node_t *curr = *head;
    client_node_t *prev = NULL;
    
    while (curr) {
        if (curr->id.ip == id.ip && curr->id.port == id.port) {
            if (prev) {
                prev->next = curr->next;
            } else {
                *head = curr->next;
            }
            free(curr);
            printf("Client [%s:%d] disconnected. Counter reset.\n", 
                   inet_ntoa(*(struct in_addr*)&id.ip), ntohs(id.port));
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}
