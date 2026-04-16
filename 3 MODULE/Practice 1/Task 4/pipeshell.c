#include "pipeshell.h"
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_CMDS 24 // максимальное количество команд в конвейере
#define MAX_ARGS 48 // максимальное количество аргументов в команде

typedef struct {
    char *argv[MAX_ARGS];
    int argc;
    char *infile;
    char *outfile;
} ParsedCmd;

static void trim_spaces(char *s) { // удаляем начальные и конечные пробельные символы
    char *end;
    while (*s == ' ' || *s == '\t')
        memmove(s, s + 1, strlen(s) + 1);
    end = s + strlen(s);
    while (end > s && (end[-1] == ' ' || end[-1] == '\t')) // отрезаем пробельные символы в конце строки
        *--end = '\0'; // ставим \0 на (место последнего непробельного символа) + 1
}

static int split_pipeline(char *line, char **segs, int max) {
    int n = 0;
    char *p = line;
    while (n < max) {
        // пропускаем начальные пробелы перед сегментом
        while (*p == ' ' || *p == '\t')
            p++;
        // если достигли конца строки заканчиваем
        if (*p == '\0')
            break;
        // сохраняем начало сегиента
        segs[n++] = p; //segs[0] = p и n+=1
        // пока не достигли конца строки или |  идем дальше
        while (*p && *p != '|')
            p++;
        // если нашли | заменяем его на \0 и переходим к следующему символу
        if (*p == '\0')
            break;
        *p++ = '\0';
    }
    return n;
}

static int parse_segment(char *seg, ParsedCmd *out) {
    trim_spaces(seg);
    out->argc = 0;
    out->infile = out->outfile = NULL;

    char *tok = strtok(seg, " \t\r\n");
    while (tok) {
        if (strcmp(tok, "<") == 0) { // 
            tok = strtok(NULL, " \t\r\n"); // следующий токен после < - имя файла для stdin
            if (!tok)
                return -1;
            out->infile = tok;
        } else if (strcmp(tok, ">") == 0) { // следующий токен после > - имя файла для stdout
            tok = strtok(NULL, " \t\r\n");
            if (!tok)
                return -1;
            out->outfile = tok;
        } else { // обычный аргумент команды
            if (out->argc >= MAX_ARGS - 1)
                return -1;
            out->argv[out->argc++] = tok;
        }
        tok = strtok(NULL, " \t\r\n"); // следующий токен для обработки
    }
    out->argv[out->argc] = NULL;
    return out->argc > 0 ? 0 : -1; // если не было найдено аргументов команды, возвращаем ошибку
}

static int try_exec(char **argv) {
    execvp(argv[0], argv);
        static char path[512];
        const char *orig = argv[0];
        if (snprintf(path, sizeof path, "./%s", orig) < (int)sizeof path) {
            argv[0] = path;
            execvp(argv[0], argv);
        }
}

static void run_pipeline(ParsedCmd *cmds, int ncmds) {
    int npipes = ncmds - 1;
    int fds[32][2];

    if (npipes > (int)(sizeof fds / sizeof fds[0])) {
        fputs("Слишком длинный конвейер.\n", stderr);
        return;
    }

    for (int i = 0; i < npipes; i++) {
        if (pipe(fds[i]) < 0) {
            perror("pipe");
            return;
        }
    }

    for (int i = 0; i < ncmds; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            break;
        }
        // в дочернем процессе
        if (pid == 0) {
            //не первой команде вместо stdin - предыдущий pipe
            if (i > 0) {
                dup2(fds[i - 1][0], STDIN_FILENO);
            } 
            //первая команда читает из файла, если он указан вместо stdin
            else if (cmds[0].infile) {
                int fd = open(cmds[0].infile, O_RDONLY);
                if (fd < 0) {
                    perror(cmds[0].infile);
                    _exit(127);
                }
                dup2(fd, STDIN_FILENO);
                close(fd);
            }
            // не последней команде вместо stdout - следующий pipe
            if (i < ncmds - 1) {
                dup2(fds[i][1], STDOUT_FILENO);
            } 
            // последняя команда пишет в файл, если он указан, вместо stdout
            else if (cmds[ncmds - 1].outfile) {
                int fd = open(cmds[ncmds - 1].outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) {
                    perror(cmds[ncmds - 1].outfile);
                    _exit(127);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }
            // закрываем все пайпы в дочернем процессе
            for (int j = 0; j < npipes; j++) {
                close(fds[j][0]); // закрывает конец для чтения
                close(fds[j][1]); // закрывает конец для записи
            }
            //  попытка выполнения команды
            const char *prog = cmds[i].argv[0];
            if (try_exec(cmds[i].argv) < 0) {
                if (errno == ENOENT)
                    fprintf(stderr, "Программа не найдена: %s\n", prog);
                else
                    fprintf(stderr, "%s: %s\n", prog, strerror(errno));
            }
            _exit(127);
        }
    }
    // в родительском процессе закрываем все пайпы
    for (int j = 0; j < npipes; j++) {
        close(fds[j][0]);
        close(fds[j][1]);
    }
    // ждем завершения всех дочерних процессов
    for (int i = 0; i < ncmds; i++)
        wait(NULL);
}

void pipeshell_loop(void) {
    char line[MAX_LINE];

    for (;;) {
        printf("pipeshell> ");
        fflush(stdout);

        if (!fgets(line, sizeof line, stdin)) {
            putchar('\n');
            break;
        }
        // удаляем \r\n в конце строки
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0')
            continue;

        char *segs[MAX_CMDS];
        char bufcopy[MAX_LINE];
        // работаем с копией строки чтобы не портить данные при разбиении на сегменты и токенизации
        strncpy(bufcopy, line, sizeof bufcopy - 1);
        bufcopy[sizeof bufcopy - 1] = '\0';

        // разбиваем строку на сегменты по символу '|'
        int nseg = split_pipeline(bufcopy, segs, MAX_CMDS);
        if (nseg == 0)
            continue;
        if (nseg >= MAX_CMDS) {
            fputs("Слишком много команд в конвейере.\n", stderr);
            continue;
        }

        ParsedCmd cmds[MAX_CMDS];
        char segwork[MAX_CMDS][MAX_LINE]; // общий массив для хранения блоков команд пайпланов и их содержимого
        int ok = 1;
        for (int i = 0; i < nseg; i++) {
            strncpy(segwork[i], segs[i], sizeof segwork[i] - 1);
            segwork[i][sizeof segwork[i] - 1] = '\0';
            if (parse_segment(segwork[i], &cmds[i]) < 0) { // парсим сегмент на команду, аргументы и перенаправления
                fputs("Ошибка разбора команды или перенаправления.\n", stderr);
                ok = 0;
                break;
            }
        }
        if (!ok)
            continue;

        run_pipeline(cmds, nseg);
    }
}
