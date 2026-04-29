#include "sniffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <linux/if_packet.h>

void hex_dump(const uint8_t *data, int size, FILE *out) {
    for (int i = 0; i < size; i++) {
        if (i > 0 && i % 16 == 0) fprintf(out, "\n");
        fprintf(out, "%02x ", data[i]);
    }
    fprintf(out, "\n\n");
}

void start_sniffing(int target_port) {
    int sockfd;
    uint8_t buffer[65536];
    FILE *log_fp = fopen(LOG_FILE, "a");

    if (!log_fp) {
        perror("fopen log");
        exit(EXIT_FAILURE);
    }

    // создаем сырой сокет для перехвата всех Ethernet кадров
    if ((sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0) {
        perror("socket");
        fclose(log_fp);
        exit(EXIT_FAILURE);
    }

    printf("Sniffer started. Target UDP port: %d\n", target_port);

    while (1) {
        ssize_t data_size = recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL);
        if (data_size < 0) {
            perror("recvfrom");
            break;
        }

        // 1. Ethernet заголовок
        struct ethhdr *eth = (struct ethhdr *)buffer;
        if (ntohs(eth->h_proto) != ETH_P_IP) continue;

        // 2. IP заголовок
        struct iphdr *ip = (struct iphdr *)(buffer + sizeof(struct ethhdr));
        if (ip->protocol != IPPROTO_UDP) continue;

        // 3. UDP заголовок
        unsigned short iphdrlen = ip->ihl * 4;
        struct udphdr *udp = (struct udphdr *)(buffer + sizeof(struct ethhdr) + iphdrlen);

        int src_port = ntohs(udp->source);
        int dst_port = ntohs(udp->dest);

        if (src_port == target_port || dst_port == target_port) { // если порт совпадает с целевым, выводим информацию о пакете
            int payload_offset = sizeof(struct ethhdr) + iphdrlen + sizeof(struct udphdr);
            int payload_size = data_size - payload_offset;
            uint8_t *payload = buffer + payload_offset;

            printf("\n--- UDP Packet ---\n");
            printf("Source: %s:%d\n", inet_ntoa(*(struct in_addr *)&ip->saddr), src_port); // выводим информацию о пакете
            printf("Dest:   %s:%d\n", inet_ntoa(*(struct in_addr *)&ip->daddr), dst_port);
            printf("Payload (%d bytes): %.*s\n", payload_size, payload_size, (char *)payload);

            fprintf(log_fp, "Packet from %s:%d to %s:%d\n", 
                    inet_ntoa(*(struct in_addr *)&ip->saddr), src_port,
                    inet_ntoa(*(struct in_addr *)&ip->daddr), dst_port);
            hex_dump(buffer, data_size, log_fp);
            fflush(log_fp);
        }
    }

    close(sockfd);
    fclose(log_fp);
}
