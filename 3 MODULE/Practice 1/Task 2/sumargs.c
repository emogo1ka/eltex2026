#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    long long s = 0;
    for (int i = 1; i < argc; i++)
        s += strtoll(argv[i], NULL, 10);
    printf("%lld\n", s);
    return 0;
}
