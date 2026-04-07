#include "minishell.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int parse_command_line(char *buf, char **argv, int max_args) {
    int n = 0;
    // разбиваем строку на токены по пробелам
    char *p = strtok(buf, " \t\r\n");
    while (p) {
        // если аргументов слишком много ошибка
        if (n >= max_args - 1)
            return -1;
        // сохраняем токен в argv и идем дальше
        argv[n++] = p;
        p = strtok(NULL, " \t\r\n");
    }
    // кладем null в конец
    argv[n] = NULL;
    return n;
}

int run_command(char **argv) {
    if (!argv || !argv[0])
        return -1;
    // по сути заменяет текущий процесс на передаваемую команду с аргументами
    execvp(argv[0], argv); // пытаемся выполнить команду

    // если execvp вернул -1 проверяем errno
    if (errno == ENOENT) {
        char path[512];
        // если команда не найдена пробуем выполнить её из текущей директории

        // int snprintf(char *str, size_t size, const char *format, ...);
        if (snprintf(path, sizeof path, "./%s", argv[0]) >= (int)sizeof path) // ошибка если имя команды слишком длинное для буфера 
            return -1;
        // вместо имени программы передаем путь и пробуем выполнить
        argv[0] = path;
        execvp(argv[0], argv);
    }

    return -1;
}
