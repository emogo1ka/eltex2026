#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>

#define SERVER_MTYPE 10L
#define MAX_TEXT 256
#define MAX_PRIORITY 1024

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

int main(void) {
    signal(SIGINT, handle_sigint);

    key_t key = ftok("/tmp", 'Q'); //генерируем уникальный id inode + device + proj_id
    if (key == (key_t)-1) {
        perror("ftok");
        return 1;
    }

    int qid = msgget(key, IPC_CREAT | 0666);
    if (qid == -1) {
        perror("msgget");
        return 1;
    }

    bool active_clients[MAX_PRIORITY] = {false};
    chat_msg_t in_msg;

    printf("Server started. Queue id: %d\n", qid);

    while (keep_running) {
        ssize_t read_size = msgrcv(qid, &in_msg, sizeof(in_msg) - sizeof(long), SERVER_MTYPE, 0); //читаем сообщения
        if (read_size == -1) {
            if (errno == EINTR) {
                continue;
            }
            perror("msgrcv");
            break;
        }

        if (in_msg.sender_priority <= 0 || in_msg.sender_priority >= MAX_PRIORITY) { // проверяем валидность приоритета
            continue;
        }

        active_clients[in_msg.sender_priority] = true; // отмечаем клиента как активного

        if (strcmp(in_msg.text, "shutdown") == 0) {
            active_clients[in_msg.sender_priority] = false;
            printf("Client %d disconnected\n", in_msg.sender_priority);
            continue;
        }

        printf("[%d] %s\n", in_msg.sender_priority, in_msg.text);

        for (int pr = 20; pr < MAX_PRIORITY; pr += 10) {
            if (!active_clients[pr] || pr == in_msg.sender_priority) {
                continue;
            }

            chat_msg_t out_msg;
            out_msg.mtype = pr;
            out_msg.sender_priority = in_msg.sender_priority;
            strncpy(out_msg.text, in_msg.text, MAX_TEXT - 1);
            out_msg.text[MAX_TEXT - 1] = '\0';

            if (msgsnd(qid, &out_msg, sizeof(out_msg) - sizeof(long), 0) == -1) {
                perror("msgsnd");
            }
        }
    }

    if (msgctl(qid, IPC_RMID, NULL) == -1) {
        perror("msgctl(IPC_RMID)");
        return 1;
    }

    printf("Server stopped.\n");
    return 0;
}
