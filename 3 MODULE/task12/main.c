#include <stdio.h>
#include <stdlib.h>
#include "udp_chat.h"

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <local_port> <remote_ip> <remote_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int local_port = atoi(argv[1]);
    const char *remote_ip = argv[2];
    int remote_port = atoi(argv[3]);

    start_chat(local_port, remote_ip, remote_port);

    return 0;
}
