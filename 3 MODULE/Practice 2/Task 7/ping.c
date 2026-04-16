#define _GNU_SOURCE
#include <errno.h>
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

    mqd_t req = mq_open(REQ_QUEUE, O_CREAT | O_WRONLY, 0666, &attr);
    if (req == (mqd_t)-1) {
        perror("mq_open req");
        return 1;
    }

    mqd_t resp = mq_open(RESP_QUEUE, O_CREAT | O_RDONLY, 0666, &attr);
    if (resp == (mqd_t)-1) {
        perror("mq_open resp");
        mq_close(req);
        return 1;
    }

    char out[MAX_TEXT];
    char in[MAX_TEXT];

    printf("ping started. Type message, 'exit' to stop.\n");
    while (fgets(out, sizeof(out), stdin) != NULL) {
        out[strcspn(out, "\n")] = '\0'; // удаляем символ новой строки

        unsigned int prio = 1; 
        if (strcmp(out, "exit") == 0) {
            prio = STOP_PRIO;
        }

        if (mq_send(req, out, strlen(out) + 1, prio) == -1) { // отправляем сообщение out в очередь req
            perror("mq_send");
            break;
        }

        if (mq_receive(resp, in, sizeof(in), &prio) == -1) { //ожидаем получения ответного сообщения из очереди resp
            perror("mq_receive");
            break;
        }

        printf("pong -> %s\n", in);
        if (prio == STOP_PRIO) {
            break;
        }
    }

    mq_close(req);
    mq_close(resp);
    return 0;
}
