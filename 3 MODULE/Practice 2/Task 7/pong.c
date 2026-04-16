#define _GNU_SOURCE
#include <fcntl.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define REQ_QUEUE "/task7_req"
#define RESP_QUEUE "/task7_resp"
#define MAX_TEXT 256
#define STOP_PRIO 99U

int main(void) {
    struct mq_attr attr;
    memset(&attr, 0, sizeof(attr));
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_TEXT;

    mqd_t req = mq_open(REQ_QUEUE, O_CREAT | O_RDONLY, 0666, &attr); // создаем очередь сообщений для получения запросов от ping
    if (req == (mqd_t)-1) {
        perror("mq_open req");
        return 1;
    }

    mqd_t resp = mq_open(RESP_QUEUE, O_CREAT | O_WRONLY, 0666, &attr); // создаем очередь сообщений для отправки ответов ping
    if (resp == (mqd_t)-1) {
        perror("mq_open resp");
        mq_close(req);
        return 1;
    }

    char in[MAX_TEXT];
    char out[MAX_TEXT];
    unsigned int prio = 0;

    printf("pong started.\n");
    while (1) {
        if (mq_receive(req, in, sizeof(in), &prio) == -1) { // ожидаем получения сообщения из очереди req
            perror("mq_receive");
            break;
        }
        printf("pong received (prio=%u): %s\n", prio, in);
        fflush(stdout);

        if (prio == STOP_PRIO) { // проверка на получение команды остановки от ping
            strcpy(out, "stop acknowledged");
            printf("pong stopping...\n");
            fflush(stdout);
            mq_send(resp, out, strlen(out) + 1, STOP_PRIO); // отправляем подтверждение остановки в очередь resp с приоритетом STOP_PRIO
            break;
        }

        snprintf(out, sizeof(out), "echo: %.249s", in); // формируем ответное сообщение
        printf("pong sent: %s\n", out);
        fflush(stdout);
        if (mq_send(resp, out, strlen(out) + 1, 1) == -1) {
            perror("mq_send");
            break;
        }
    }

    mq_close(req);
    mq_close(resp);
    mq_unlink(REQ_QUEUE);
    mq_unlink(RESP_QUEUE);
    return 0;
}
