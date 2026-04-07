#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "net.h"

#ifdef _WIN32
#include <windows.h>
#endif

static void usage(const char *argv0) {
    fprintf(stderr, "использование: %s <шлюз> <маска|/префикс> <N>\n", argv0);
    fprintf(stderr, "пример: %s 192.168.0.1 255.255.255.0 1000\n", argv0);
    fprintf(stderr, "пример: %s 192.168.0.1 /24 1000\n", argv0);
}

static int parse_mask_arg(const char *arg, uint32_t *out_mask) {
    if (arg[0] == '/') {
        char *end = NULL;
        long p = strtol(arg + 1, &end, 10);
        if (!end || *end != '\0' || p < 0 || p > 32)
            return -1;
        if (p == 0) {
            *out_mask = 0U;
        } else {
            *out_mask = 0xFFFFFFFFU << (32 - (unsigned)p);
        }
        return 0;
    }
    return net_parse_ipv4(arg, out_mask);
}


// (gw & mask) == (dest & mask)
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
    if (net_parse_ipv4(argv[1], &gw) != 0 || parse_mask_arg(argv[2], &mask) != 0) {
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
