#include "counter.h"
#include <stdio.h>

int main(int argc, char **argv) {
    const char *path = (argc > 1) ? argv[1] : "counter.log";
    counter_run(path);
    return 0;
}
