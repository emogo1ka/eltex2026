#include "taxi.h"

void get_fifo_names(pid_t pid, char *req, char *resp) {
    sprintf(req, "/tmp/taxi_req_%d", pid);
    sprintf(resp, "/tmp/taxi_resp_%d", pid);
}

int parse_command(char *line, int *pid, int *timer) {
    if (strncmp(line, "create_driver", 13) == 0) return CMD_CREATE_DRIVER;
    if (strncmp(line, "quit", 4) == 0) return CMD_QUIT;
    if (strncmp(line, "get_drivers", 11) == 0) return CMD_GET_DRIVERS;
    
    if (sscanf(line, "send_task %d %d", pid, timer) == 2) return CMD_SEND_TASK;
    if (sscanf(line, "get_status %d", pid) == 1) return CMD_GET_STATUS;
    
    return CMD_UNKNOWN;
}

void driver_loop(pid_t pid) { // основной цикл водителя
    char req_fifo[64], resp_fifo[64];
    get_fifo_names(pid, req_fifo, resp_fifo);

    mkfifo(req_fifo, 0666);
    mkfifo(resp_fifo, 0666);

    // CLI открывает REQ для записи и RESP для чтения
    // водимтель открывает REQ для чтения и RESP для записи
    int req_fd = open(req_fifo, O_RDONLY | O_NONBLOCK);
    int resp_fd = open(resp_fifo, O_WRONLY);

    time_t task_end = 0;
    int is_busy = 0;

    while (1) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(req_fd, &readfds); // ждем команды от CLI

        struct timeval tv;
        struct timeval *tv_ptr = NULL;

        if (is_busy) { // если водитель занят, то устанавливаем таймаут до окончания задачи
            time_t now = time(NULL);
            if (now >= task_end) {
                is_busy = 0;
            } else {
                tv.tv_sec = task_end - now;
                tv.tv_usec = 0;
                tv_ptr = &tv;
            }
        }

        int ret = select(req_fd + 1, &readfds, NULL, NULL, tv_ptr); // ждем команды с консоли или окончания задачи

        if (ret < 0) {
            if (errno == EINTR) continue;
            break;
        }

        if (ret == 0) { // окончание задачи
            is_busy = 0;
            write(resp_fd, "TASK COMPLETED", 14);
            continue;
        }

        if (FD_ISSET(req_fd, &readfds)) {
            char buf[BUFFER_SIZE];
            ssize_t n = read(req_fd, buf, BUFFER_SIZE - 1);
            if (n <= 0) continue; // EOF или ошибка
            buf[n] = 0;

            char response[BUFFER_SIZE];
            if (strncmp(buf, "STATUS", 6) == 0) {
                if (is_busy) {
                    sprintf(response, "Busy %lds", (long)(task_end - time(NULL)));
                } else {
                    sprintf(response, "Available");
                }
            } else if (strncmp(buf, "TASK", 4) == 0) {
                int duration = atoi(buf + 5);
                if (is_busy) {
                    sprintf(response, "Busy %lds", (long)(task_end - time(NULL)));
                } else {
                    is_busy = 1;
                    task_end = time(NULL) + duration;
                    sprintf(response, "Started task for %ds", duration);
                }
            } else {
                sprintf(response, "Unknown Command");
            }
            write(resp_fd, response, strlen(response));
        }
    }

    close(req_fd);
    close(resp_fd);
    unlink(req_fifo);
    unlink(resp_fifo);
    exit(0);
}
