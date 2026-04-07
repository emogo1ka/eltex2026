#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        fputs("maxnum: нужны числовые аргументы\n", stderr);
        return 1;
    }
    long long m = strtoll(argv[1], NULL, 10);
    for (int i = 2; i < argc; i++) {
        long long v = strtoll(argv[i], NULL, 10);
        if (v > m)
            m = v;
    }
    printf("%lld\n", m);
    return 0;
}
