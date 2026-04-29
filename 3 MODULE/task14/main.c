#include <stdio.h>
#include <stdlib.h>
#include "sniffer.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: sudo %s <target_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    start_sniffing(port);

    return 0;
}
