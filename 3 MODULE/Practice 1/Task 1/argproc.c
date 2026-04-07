#include "argproc.h"
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int parse_number(const char *s, double *out) {
    if (!s || !*s)
        return 0;

    char *end = NULL;
    errno = 0;
    double v = strtod(s, &end); // строку в double
    if (errno == ERANGE) // проверка размера числа
        return 0;
    if (end == s) // проверка наличия числовых символов
        return 0;
    while (*end && isspace((unsigned char)*end)) // пропускаем пробельные символы в конце строки
        end++;
    if (*end != '\0')
        return 0;

    *out = v;
    return 1;
}

void process_argument(const char *arg) { // если аргумент число печатаем его и удвоенное значение иначе просто печатаем аргумент
    double v;
    if (parse_number(arg, &v)) {
        if (floor(v) == v && fabs(v) < 1e15)
            printf("%lld %lld\n", (long long)v, (long long)(v * 2.0));
        else
            printf("%.15g %.15g\n", v, v * 2.0);
    } else {
        puts(arg);
    }
    fflush(stdout);
}

void process_argument_range(char **argv, int start, int end) {
    if (!argv || start > end)
        return;
    for (int i = start; i <= end; i++)
        process_argument(argv[i]);
}
