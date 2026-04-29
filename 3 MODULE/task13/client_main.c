#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "protocol.h"

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
    int my_sock, portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

    portno = atoi(argv[2]);
    my_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (my_sock < 0) error("ERROR opening socket");

    server = gethostbyname(argv[1]);
    if (server == NULL) error("ERROR, no such host");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(my_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    printf("Connected to server %s:%d\n", argv[1], portno);

    while (1) {
        printf("\n--- MENU ---\n1. Math Operation (+, -, *, /)\n2. Download File\n0. Exit\nChoice: ");
        int choice;
        if (scanf("%d", &choice) <= 0) break;
        write(my_sock, &choice, sizeof(choice));

        if (choice == OP_EXIT) break;

        if (choice == OP_MATH) {
            char op, val_a[64], val_b[64];
            printf("Enter operation (+, -, *, /): "); scanf(" %c", &op);
            write(my_sock, &op, 1);
            
            printf("Enter first number: "); scanf("%s", val_a);
            write(my_sock, val_a, sizeof(val_a));
            
            printf("Enter second number: "); scanf("%s", val_b);
            write(my_sock, val_b, sizeof(val_b));

            char response[BUFFER_SIZE];
            memset(response, 0, sizeof(response));
            read(my_sock, response, sizeof(response));
            printf("SERVER: %s", response);
        } 
        else if (choice == OP_FILE) {
            char filename[256];
            printf("Enter filename to download: "); scanf("%s", filename);
            write(my_sock, filename, sizeof(filename));

            long fsize;
            read(my_sock, &fsize, sizeof(fsize));
            if (fsize < 0) {
                printf("Error: File not found on server.\n");
                continue;
            }

            printf("Downloading %ld bytes...\n", fsize);
            FILE *fp = fopen("downloaded_file", "wb");
            char buffer[BUFFER_SIZE];
            long total = 0;
            while (total < fsize) {
                int n = read(my_sock, buffer, BUFFER_SIZE);
                if (n <= 0) break;
                fwrite(buffer, 1, n, fp);
                total += n;
            }
            fclose(fp);
            printf("File received successfully (saved as 'downloaded_file')\n");
        }
    }

    close(my_sock);
    return 0;
}
