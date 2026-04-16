#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define SERVER_MTYPE 10L
#define MAX_TEXT 256

typedef struct {
    long mtype;
    int sender_priority;
    char text[MAX_TEXT];
} chat_msg_t;

static volatile sig_atomic_t keep_running = 1;

static void handle_sigint(int signum) {
    (void)signum;
    keep_running = 0;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <client_priority: 20,30,40,...>\n", argv[0]);
        return 1;
    }

    int my_priority = atoi(argv[1]);
    if (my_priority < 20 || my_priority % 10 != 0) {
        fprintf(stderr, "Priority must be 20, 30, 40, ...\n");
        return 1;
    }

    signal(SIGINT, handle_sigint);

    key_t key = ftok("/tmp", 'Q'); //генерируем уникальный id inode + device + proj_id
    if (key == (key_t)-1) {
        perror("ftok");
        return 1;
    }

    int qid = msgget(key, 0666); //создаем очередь сообщений с правами доступа 0666
    if (qid == -1) {
        perror("msgget");
        return 1;
    }

    pid_t reader_pid = fork(); //создаем дочерний процесс для чтения сообщений
    if (reader_pid == -1) {
        perror("fork");
        return 1;
    }

    if (reader_pid == 0) {
        chat_msg_t in_msg;
        while (keep_running) {
            //читаем сообщения с типом, равным приоритету клиента
            ssize_t read_size = msgrcv(qid, &in_msg, sizeof(in_msg) - sizeof(long), my_priority, 0); 
            if (read_size == -1) {
                if (errno == EINTR) {
                    if (!keep_running) {
                        break;   // выходим по сигналу завершения
                    }
                    continue;    // если прерывание, но не сигнал завершения, продолжаем ждать
                }
                perror("msgrcv");
                break;
            }
            printf("[%d] %s\n", in_msg.sender_priority, in_msg.text);
            fflush(stdout);
        }
        _exit(0);
    }

    char buffer[MAX_TEXT];
    while (keep_running && fgets(buffer, sizeof(buffer), stdin) != NULL) {
        buffer[strcspn(buffer, "\n")] = '\0';

        chat_msg_t out_msg;
        out_msg.mtype = SERVER_MTYPE; // поле структуры для типа сообщения - сообщение для сервера
        out_msg.sender_priority = my_priority;
        strncpy(out_msg.text, buffer, MAX_TEXT - 1);
        out_msg.text[MAX_TEXT - 1] = '\0';

        if (msgsnd(qid, &out_msg, sizeof(out_msg) - sizeof(long), 0) == -1) { //отправляем сообщение на сервер
            perror("msgsnd");
            break;
        }

        if (strcmp(buffer, "shutdown") == 0) {
            break;
        }
    }

    keep_running = 0;
    kill(reader_pid, SIGINT);
    waitpid(reader_pid, NULL, 0);
    return 0;
}
