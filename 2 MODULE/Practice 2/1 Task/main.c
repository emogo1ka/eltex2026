#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "perm.h"

#ifdef _WIN32
#include <windows.h>
#endif

static void trim_newline(char *s) {
    size_t n = strlen(s);
    while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r')) {
        s[n - 1] = '\0';
        n--;
    }
}

static int input_looks_octal(const char *s) {
    const char *p = s;
    while (*p == ' ' || *p == '\t')
        p++;
    if (*p == '\0')
        return 0;
    for (; *p; p++) {
        if (*p == ' ' || *p == '\t')
            continue;
        if (*p < '0' || *p > '7')
            return 0;
    }
    return 1;
}

int main(void) {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
#endif

    unsigned current = 0644U;
    int choice;

    for (;;) {
        printf("\n1 — ввести права и показать представления\n");
        printf("2 — права файла через stat\n");
        printf("3 — chmod к текущим правам, без записи в файл\n");
        printf("0 — выход\n");
        printf("выбор: ");
        if (scanf("%d", &choice) != 1) {
            printf("нужно число\n");
            while (getchar() != '\n')
                ;
            continue;
        }
        while (getchar() != '\n')
            ;

        if (choice == 0) {
            printf("выход\n");
            return 0;
        }

        if (choice == 1) {
            char line[256];
            printf("права (например 755 или rwxr-xr-x): ");
            if (!fgets(line, sizeof(line), stdin)) {
                printf("ошибка ввода\n");
                continue;
            }
            trim_newline(line);
            if (input_looks_octal(line)) {
                current = perm_parse_octal(line) & 0777U;
            } else {
                unsigned m;
                if (perm_parse_rwx_string(line, &m) != 0) {
                    printf("не удалось разобрать. буквенно: 9 символов подряд без дефисов, как rwxrwxrwx\n");
                    continue;
                }
                current = m & 0777U;
            }
            perm_print_all(current);
        } else if (choice == 2) {
            char path[512];
            printf("путь к файлу (например perm.exe): ");
            if (!fgets(path, sizeof(path), stdin)) {
                printf("ошибка ввода\n");
                continue;
            }
            trim_newline(path);
            if (perm_stat_path(path, &current) != 0)
                continue;
            perm_print_all(current);
        } else if (choice == 3) {
            char spec[512];
            printf("chmod (например u+x или 644): ");
            if (!fgets(spec, sizeof(spec), stdin)) {
                printf("ошибка ввода\n");
                continue;
            }
            trim_newline(spec);
            current = perm_apply_chmod(current, spec) & 0777U;
            perm_print_all(current);
        } else {
            printf("нет такого пункта\n");
        }
    }
}
