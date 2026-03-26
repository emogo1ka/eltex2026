#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "net.h"

#ifdef _WIN32
#include <windows.h>
#endif

static void usage(const char *argv0) {
    fprintf(stderr, "использование: %s <шлюз> <маска> <N>\n", argv0);
}

int main(int argc, char **argv) {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
#endif

    if (argc != 4) {
        usage(argv[0]);
        return 1;
    }

    uint32_t gw, mask;
    if (net_parse_ipv4(argv[1], &gw) != 0 || net_parse_ipv4(argv[2], &mask) != 0) {
        fprintf(stderr, "неверный ip или маска\n");
        return 1;
    }

    char *end = NULL;
    long nlong = strtol(argv[3], &end, 10);
    if (!end || *end != '\0' || nlong < 0 || nlong > 100000000L) {
        fprintf(stderr, "N должно быть неотрицательным числом\n");
        return 1;
    }
    unsigned long N = (unsigned long)nlong;

    srand((unsigned)time(NULL));

    unsigned long local_cnt = 0;
    unsigned long other_cnt = 0;

    for (unsigned long i = 0; i < N; i++) {
        uint32_t dest = net_random_ip();
        if (net_same_subnet(gw, mask, dest))
            local_cnt++;
        else
            other_cnt++;
    }

    double pct_local = N ? (100.0 * (double)local_cnt / (double)N) : 0.0;
    double pct_other = N ? (100.0 * (double)other_cnt / (double)N) : 0.0;

    printf("обработано пакетов: %lu\n", N);
    printf("своей подсети: %lu (%.2f%%)\n", local_cnt, pct_local);
    printf("других сетей: %lu (%.2f%%)\n", other_cnt, pct_other);

    return 0;
}
