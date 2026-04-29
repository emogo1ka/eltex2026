#ifndef UDP_CHAT_H
#define UDP_CHAT_H

#include <netinet/in.h>

#define MAX_MSG_LEN 1024

typedef struct {
    int sockfd;
    struct sockaddr_in peer_addr;
} chat_config_t;

void *receive_thread(void *arg);
void start_chat(int local_port, const char *remote_ip, int remote_port);

#endif // UDP_CHAT_H
