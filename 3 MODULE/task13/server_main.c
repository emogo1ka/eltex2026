#define _DEFAULT_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "protocol.h"

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void handle_file_transfer(int sock) {
    char filename[256];
    char buffer[BUFFER_SIZE];
    
    if (read(sock, filename, sizeof(filename)) <= 0) return;
    printf("[Server] Client requested file: %s\n", filename);
    
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        long error_size = -1;
        write(sock, &error_size, sizeof(error_size));
        return;
    }

    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    rewind(fp);
    write(sock, &fsize, sizeof(fsize));

    int n;
    while ((n = fread(buffer, 1, BUFFER_SIZE, fp)) > 0) {
        write(sock, buffer, n);
    }
    fclose(fp);
    printf("[Server] File %s sent successfully\n", filename);
}

void dostuff(int sock) {
    char buff[BUFFER_SIZE];
    int cmd_code;

    while (read(sock, &cmd_code, sizeof(cmd_code)) > 0) {
        if (cmd_code == OP_EXIT) break;

        if (cmd_code == OP_FILE) {
            handle_file_transfer(sock);
            continue;
        }

        if (cmd_code == OP_MATH) {
            char op;
            char val_a[64], val_b[64];
            double a, b, res;
            
            read(sock, &op, 1);
            read(sock, val_a, sizeof(val_a)); a = atof(val_a);
            read(sock, val_b, sizeof(val_b)); b = atof(val_b);

            switch (op) {
                case '+': res = a + b; break;
                case '-': res = a - b; break;
                case '*': res = a * b; break;
                case '/': res = (b != 0) ? a / b : 0; break;
                default: res = 0;
            }

            memset(buff, 0, sizeof(buff));
            snprintf(buff, sizeof(buff), "Result: %.2f\n", res);
            write(sock, buff, strlen(buff) + 1);
        }
    }
    printf("[Server] Client disconnected\n");
    close(sock);
}

int main(int argc, char *argv[]) {
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    portno = (argc < 2) ? DEFAULT_PORT : atoi(argv[1]);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");

    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    printf("TCP SERVER DEMO (Lecture based) on port %d\n", portno);

    // Очистка зомби-процессов
    signal(SIGCHLD, SIG_IGN);

    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0) error("ERROR on accept");
        
        printf("[Server] + New connection from %s\n", inet_ntoa(cli_addr.sin_addr));

        int pid = fork();
        if (pid < 0) error("ERROR on fork");
        if (pid == 0) {
            close(sockfd);
            dostuff(newsockfd);
            exit(0);
        } else {
            close(newsockfd);
        }
    }
    return 0;
}
