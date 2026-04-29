#include "tcp_logic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/stat.h>

int setup_server(int port) {
    int sockfd;
    struct sockaddr_in addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return -1;
    }

    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(sockfd);
        return -1;
    }

    if (listen(sockfd, 5) < 0) {
        perror("listen");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

int connect_to_server(const char *ip, int port) {
    int sockfd;
    struct sockaddr_in addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0) {
        perror("inet_pton");
        return -1;
    }

    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect");
        return -1;
    }

    return sockfd;
}

void handle_client(int client_fd) {
    packet_t pkt;
    if (recv(client_fd, &pkt, sizeof(pkt), 0) <= 0) return;

    packet_t response;
    memset(&response, 0, sizeof(response));

    if (pkt.type == OP_FILE_TRANSFER) {
        printf("Server: Receiving file %s (%ld bytes)\n", pkt.payload.file_meta.filename, pkt.payload.file_meta.size);
        
        char save_path[300];
        snprintf(save_path, sizeof(save_path), "uploaded_%s", pkt.payload.file_meta.filename);
        
        FILE *fp = fopen(save_path, "wb");
        if (!fp) {
            perror("fopen");
            return;
        }

        long remaining = pkt.payload.file_meta.size;
        char buffer[CHUNK_SIZE];
        while (remaining > 0) {
            ssize_t n = recv(client_fd, buffer, (remaining < CHUNK_SIZE) ? remaining : CHUNK_SIZE, 0);
            if (n <= 0) break;
            fwrite(buffer, 1, n, fp);
            remaining -= n;
        }
        fclose(fp);
        printf("Server: File saved as %s\n", save_path);
        
        response.type = OP_RESULT;
        response.payload.result.value = 0; // Success indicator
    } else {
        response.type = OP_RESULT;
        double a = pkt.payload.math.a;
        double b = pkt.payload.math.b;

        switch (pkt.type) {
            case OP_ADD: response.payload.result.value = a + b; break;
            case OP_SUB: response.payload.result.value = a - b; break;
            case OP_MUL: response.payload.result.value = a * b; break;
            case OP_DIV: 
                if (b != 0) response.payload.result.value = a / b;
                else {
                    response.type = OP_ERROR;
                    strcpy(response.payload.error_msg, "Division by zero");
                }
                break;
            default:
                response.type = OP_ERROR;
                strcpy(response.payload.error_msg, "Unknown operation");
        }
        printf("Server: Processed operation %d, result: %f\n", pkt.type, response.payload.result.value);
    }

    send(client_fd, &response, sizeof(response), 0);
}

void send_math_request(int sockfd, op_type_t type, double a, double b) {
    packet_t pkt;
    pkt.type = type;
    pkt.payload.math.a = a;
    pkt.payload.math.b = b;
    
    send(sockfd, &pkt, sizeof(pkt), 0);
    
    packet_t res;
    if (recv(sockfd, &res, sizeof(res), 0) > 0) {
        if (res.type == OP_RESULT) {
            printf("Client: Result = %f\n", res.payload.result.value);
        } else if (res.type == OP_ERROR) {
            printf("Client: Error: %s\n", res.payload.error_msg);
        }
    }
}

void send_file(int sockfd, const char *filepath) {
    FILE *fp = fopen(filepath, "rb");
    if (!fp) {
        perror("fopen");
        return;
    }

    struct stat st;
    stat(filepath, &st);

    packet_t pkt;
    pkt.type = OP_FILE_TRANSFER;
    const char *basename = strrchr(filepath, '/');
    basename = basename ? basename + 1 : filepath;
    strncpy(pkt.payload.file_meta.filename, basename, 255);
    pkt.payload.file_meta.size = st.st_size;

    send(sockfd, &pkt, sizeof(pkt), 0);

    char buffer[CHUNK_SIZE];
    size_t n;
    while ((n = fread(buffer, 1, CHUNK_SIZE, fp)) > 0) {
        send(sockfd, buffer, n, 0);
    }
    fclose(fp);

    packet_t res;
    recv(sockfd, &res, sizeof(res), 0);
    printf("Client: File transfer complete\n");
}
