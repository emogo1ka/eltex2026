#include "taxi.h"

driver_info_t drivers[MAX_DRIVERS];
int driver_count = 0;

void cleanup_driver(int index) {
    close(drivers[index].req_fd);
    close(drivers[index].resp_fd);
    unlink(drivers[index].req_fifo);
    unlink(drivers[index].resp_fifo);
    // удаляем драйвера из массива, сдвигая остальных влево
    for (int i = index; i < driver_count - 1; i++) {
        drivers[i] = drivers[i + 1];
    }
    driver_count--;
}

int main() {
    char line[BUFFER_SIZE];
    printf("Taxi Management started.\n");
    printf("Commands: create_driver, send_task <pid> <sec>, get_status <pid>, get_drivers, quit\n");

    while (1) {
        printf("> ");
        fflush(stdout);

        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds); // добавляем stdin для чтения команд пользователя
        
        int max_fd = STDIN_FILENO;
        for (int i = 0; i < driver_count; i++) {
            FD_SET(drivers[i].resp_fd, &readfds); // добавляем каналы для чтения ответов от водителей
            if (drivers[i].resp_fd > max_fd) max_fd = drivers[i].resp_fd;
        }

        int ret = select(max_fd + 1, &readfds, NULL, NULL, NULL);
        if (ret < 0) {
            if (errno == EINTR) continue;
            perror("select");
            break;
        }

        // Handle driver responses
        for (int i = 0; i < driver_count; i++) {
            if (FD_ISSET(drivers[i].resp_fd, &readfds)) {
                char buf[BUFFER_SIZE];
                ssize_t n = read(drivers[i].resp_fd, buf, BUFFER_SIZE - 1);
                if (n > 0) {
                    buf[n] = 0;
                    printf("\n[Driver %d] Response: %s\n> ", drivers[i].pid, buf);
                    fflush(stdout);
                } else if (n == 0) {
                    printf("\n[Driver %d] Process terminated.\n> ", drivers[i].pid);
                    cleanup_driver(i);
                    i--; // Adjust index
                    fflush(stdout);
                }
            }
        }

        // обработка команд пользователя
        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            if (!fgets(line, BUFFER_SIZE, stdin)) break;
            line[strcspn(line, "\n")] = 0;

            int pid_arg, timer_arg;
            command_type_t cmd = parse_command(line, &pid_arg, &timer_arg); // парсер команд CLI

            switch (cmd) {
                case CMD_CREATE_DRIVER: {
                    if (driver_count >= MAX_DRIVERS) {
                        printf("Max drivers reached.\n");
                        break;
                    }
                    pid_t pid = fork(); // создаем новый процесс для водителя
                    if (pid == -1) {
                        perror("fork");
                    } else if (pid == 0) {
                        driver_loop(getpid());
                    } else {
                        printf("Created driver with PID: %d\n", pid);
                        char req[64], resp[64];
                        get_fifo_names(pid, req, resp);
                        
                        // создаем именованные каналы для общения с водителем
                        mkfifo(req, 0666);
                        mkfifo(resp, 0666);

                        drivers[driver_count].pid = pid;
                        strcpy(drivers[driver_count].req_fifo, req);
                        strcpy(drivers[driver_count].resp_fifo, resp);
                        
                        // CLI открывает REQ для записи и RESP для чтения, водитель открывает REQ для чтения и RESP для записи
                        drivers[driver_count].req_fd = open(req, O_WRONLY);
                        drivers[driver_count].resp_fd = open(resp, O_RDONLY | O_NONBLOCK); // O_NONBLOCK для немедленного возврата при отсутствии данных
                        
                        driver_count++;
                    }
                    break;
                }
                case CMD_SEND_TASK: {
                    int found = 0;
                    for (int i = 0; i < driver_count; i++) {
                        if (drivers[i].pid == pid_arg) {
                            char cmd_buf[32];
                            sprintf(cmd_buf, "TASK %d", timer_arg);
                            write(drivers[i].req_fd, cmd_buf, strlen(cmd_buf));
                            found = 1;
                            break;
                        }
                    }
                    if (!found) printf("Driver %d not found.\n", pid_arg);
                    break;
                }
                case CMD_GET_STATUS: {
                    int found = 0;
                    for (int i = 0; i < driver_count; i++) {
                        if (drivers[i].pid == pid_arg) {
                            write(drivers[i].req_fd, "STATUS", 6);
                            found = 1;
                            break;
                        }
                    }
                    if (!found) printf("Driver %d not found.\n", pid_arg);
                    break;
                }
                case CMD_GET_DRIVERS: {
                    if (driver_count == 0) {
                        printf("No active drivers.\n");
                    } else {
                        printf("Requesting status from all drivers...\n");
                        for (int i = 0; i < driver_count; i++) {
                            write(drivers[i].req_fd, "STATUS", 6);
                        }
                    }
                    break;
                }
                case CMD_QUIT:
                    goto end;
                case CMD_UNKNOWN:
                    if (strlen(line) > 0) printf("Unknown command.\n");
                    break;
            }
        }
    }

end:
    for (int i = 0; i < driver_count; i++) {
        kill(drivers[i].pid, SIGTERM);
        cleanup_driver(i);
    }
    return 0;
}
