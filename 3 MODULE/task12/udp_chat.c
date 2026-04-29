#include "udp_chat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

void *receive_thread(void *arg) {
    chat_config_t *config = (chat_config_t *)arg;
    char buffer[MAX_MSG_LEN];
    struct sockaddr_in sender_addr;
    socklen_t addr_len = sizeof(sender_addr);

    while (1) {
        ssize_t n = recvfrom(config->sockfd, buffer, MAX_MSG_LEN - 1, 0,
                             (struct sockaddr *)&sender_addr, &addr_len);
        if (n < 0) {
            perror("recvfrom");
            break;
        }
        buffer[n] = '\0';
        
        // Simple check to see if it's from our expected peer (optional for p2p)
        printf("\r[Peer]: %s\nYou: ", buffer);
        fflush(stdout);
    }
    return NULL;
}

void start_chat(int local_port, const char *remote_ip, int remote_port) {
    int sockfd;
    struct sockaddr_in local_addr, remote_addr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = INADDR_ANY;
    local_addr.sin_port = htons(local_port);

    if (bind(sockfd, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
        perror("bind");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    memset(&remote_addr, 0, sizeof(remote_addr));
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(remote_port);
    if (inet_pton(AF_INET, remote_ip, &remote_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    chat_config_t config = {sockfd, remote_addr};
    pthread_t recv_tid;
    
    if (pthread_create(&recv_tid, NULL, receive_thread, &config) != 0) {
        perror("pthread_create");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    char message[MAX_MSG_LEN];
    printf("Chat started. Local port: %d, Remote: %s:%d\n", local_port, remote_ip, remote_port);
    printf("You: ");
    fflush(stdout);

    while (fgets(message, MAX_MSG_LEN, stdin)) {
        message[strcspn(message, "\n")] = 0;
        if (strlen(message) == 0) continue;

        if (sendto(sockfd, message, strlen(message), 0,
                   (struct sockaddr *)&remote_addr, sizeof(remote_addr)) < 0) {
            perror("sendto");
        }
        printf("You: ");
        fflush(stdout);
    }

    close(sockfd);
}
