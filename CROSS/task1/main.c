#include "udp_raw.h"

// флаг для остановки сервера при получении сигнала
volatile sig_atomic_t stop = 0;
void handle_signal(int sig) { stop = 1; }

void run_server(int port);
void run_client(const char *ip, int port);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <server|client> [args...]\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "server") == 0) { // запуск в режиме сервера
        if (argc < 3) {
            fprintf(stderr, "Usage: %s server <port>\n", argv[0]);
            return EXIT_FAILURE;
        }
        run_server(atoi(argv[2]));
    } else if (strcmp(argv[1], "client") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Usage: %s client <ip> <port>\n", argv[0]);
            return EXIT_FAILURE;
        }
        run_client(argv[2], atoi(argv[3]));
    } else {
        fprintf(stderr, "Unknown mode: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void run_server(int port) {
    int sockfd;
    char buffer[IP_MAXPACKET];
    struct sockaddr_in servaddr, cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    client_node_t *clients = NULL;

    // создаем сырой сокет для прослушивания UDP пакетов
    if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on raw socket (UDP port %d)...\n", port);

    while (!stop) {
        ssize_t n = recvfrom(sockfd, buffer, IP_MAXPACKET, 0, (struct sockaddr *)&cliaddr, &clilen);
        if (n < 0) {
            if (errno == EINTR) continue;
            perror("recvfrom failed");
            break;
        }

        // Получаем доступ к IP-заголовку, записывая буфер как структуру iphdr
        struct iphdr *iph = (struct iphdr *)buffer;
        int iph_len = iph->ihl * 4;

        // вычисляет указатель на начало UDP-заголовка в сетевом пакете
        struct udphdr *udph = (struct udphdr *)(buffer + iph_len);
        
        // проверяем, что это UDP пакет для нашего порта
        if (ntohs(udph->dest) != port) continue;

        char *payload = buffer + iph_len + sizeof(struct udphdr);
        int payload_len = n - iph_len - sizeof(struct udphdr);
        payload[payload_len] = '\0';

        client_id_t cid = {iph->saddr, udph->source};
        
        if (strncmp(payload, SHUTDOWN_MSG, strlen(SHUTDOWN_MSG)) == 0) {
            remove_client(&clients, cid);
            continue;
        }

        client_node_t *client = find_or_create_client(&clients, cid);
        client->count++;

        printf("Received: '%s' from %s:%d (%d)\n", payload, inet_ntoa(*(struct in_addr*)&cid.ip), ntohs(cid.port), client->count);

        // подготовка ответа клиенту
        char send_buf[IP_MAXPACKET];
        char resp_payload[MAX_PAYLOAD];
        int resp_payload_len = snprintf(resp_payload, MAX_PAYLOAD, "%s %d", payload, client->count); // формируем UDP-заголовок для ответа

        struct udphdr *resp_udph = (struct udphdr *)send_buf;
        resp_udph->source = udph->dest;
        resp_udph->dest = udph->source;
        resp_udph->len = htons(sizeof(struct udphdr) + resp_payload_len); // длина UDP-заголовка + полезной нагрузки
        resp_udph->check = 0;

        // подготовка псевдозаголовка для расчета контрольной суммы UDP
        struct pseudo_header psh;
        psh.source_address = iph->daddr;
        psh.dest_address = iph->saddr;
        psh.placeholder = 0;
        psh.protocol = IPPROTO_UDP;
        psh.udp_length = resp_udph->len;

        int psize = sizeof(struct pseudo_header) + sizeof(struct udphdr) + resp_payload_len;
        char *pseudogram = malloc(psize);
        memcpy(pseudogram, (char*)&psh, sizeof(struct pseudo_header));
        memcpy(pseudogram + sizeof(struct pseudo_header), resp_udph, sizeof(struct udphdr));
        memcpy(pseudogram + sizeof(struct pseudo_header) + sizeof(struct udphdr), resp_payload, resp_payload_len);
        resp_udph->check = calculate_checksum((unsigned short*)pseudogram, psize);
        free(pseudogram);

        memcpy(send_buf + sizeof(struct udphdr), resp_payload, resp_payload_len);

        struct sockaddr_in target;
        target.sin_family = AF_INET;
        target.sin_port = udph->source;
        target.sin_addr.s_addr = iph->saddr;

        if (sendto(sockfd, send_buf, sizeof(struct udphdr) + resp_payload_len, 0, (struct sockaddr *)&target, sizeof(target)) < 0) {
            perror("sendto failed");
        }
    }
    close(sockfd);
}

void run_client(const char *ip, int port) {
    int sockfd;
    char buffer[IP_MAXPACKET];
    struct sockaddr_in servaddr;
    struct sigaction sa;

    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &servaddr.sin_addr);

    srand(time(NULL));
    uint16_t src_port = htons(1024 + rand() % 60000);

    while (!stop) {
        char msg[MAX_PAYLOAD];
        printf("Enter message: ");
        fflush(stdout); 
        if (!fgets(msg, MAX_PAYLOAD, stdin)) break;
        msg[strcspn(msg, "\n")] = 0;

        if (strlen(msg) == 0) continue;

        // сформируем UDP пакет
        char send_buf[IP_MAXPACKET];
        struct udphdr *udph = (struct udphdr *)send_buf;
        udph->source = src_port;
        udph->dest = htons(port);
        udph->len = htons(sizeof(struct udphdr) + strlen(msg));
        udph->check = 0;

        memcpy(send_buf + sizeof(struct udphdr), msg, strlen(msg));

        if (sendto(sockfd, send_buf, sizeof(struct udphdr) + strlen(msg), 0, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
            perror("sendto failed");
            continue;
        }

        // ожидаем ответ от сервера
        struct timeval tv;
        tv.tv_sec = 2;
        tv.tv_usec = 0;
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        ssize_t n = recvfrom(sockfd, buffer, IP_MAXPACKET, 0, NULL, NULL);
        if (n > 0) {
            struct iphdr *iph = (struct iphdr *)buffer;
            struct udphdr *rudph = (struct udphdr *)(buffer + iph->ihl * 4);
            if (ntohs(rudph->dest) == ntohs(src_port)) {
                char *reply = buffer + iph->ihl * 4 + sizeof(struct udphdr);
                int rlen = n - (iph->ihl * 4) - sizeof(struct udphdr);
                reply[rlen] = '\0';
                printf("Server reply: %s\n", reply);
            }
        } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("recvfrom error");
        }
    }

    // отправляем сообщение о завершении работы серверу
    printf("\nClosing connection...\n");
    char shut_buf[IP_MAXPACKET];
    struct udphdr *sudph = (struct udphdr *)shut_buf;
    sudph->source = src_port;
    sudph->dest = htons(port);
    sudph->len = htons(sizeof(struct udphdr) + strlen(SHUTDOWN_MSG));
    sudph->check = 0;
    memcpy(shut_buf + sizeof(struct udphdr), SHUTDOWN_MSG, strlen(SHUTDOWN_MSG));
    sendto(sockfd, shut_buf, sizeof(struct udphdr) + strlen(SHUTDOWN_MSG), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));

    close(sockfd);
}
