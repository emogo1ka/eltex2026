#include "epoll_server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void close_client(int epoll_fd, client_state_t *state) {
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, state->fd, NULL);
    if (state->fp) fclose(state->fp);
    close(state->fd);
    free(state);
}

void handle_math(int fd, packet_t *pkt) {
    packet_t response;
    memset(&response, 0, sizeof(response));
    response.type = OP_RESULT;
    
    double a = pkt->payload.math.a;
    double b = pkt->payload.math.b;

    switch (pkt->type) {
        case OP_ADD: response.payload.result.value = a + b; break;
        case OP_SUB: response.payload.result.value = a - b; break;
        case OP_MUL: response.payload.result.value = a * b; break;
        case OP_DIV: 
            if (b != 0) response.payload.result.value = a / b;
            else {
                response.type = OP_ERROR;
                strcpy(response.payload.error_msg, "Division by zero");
            }
            break;
        default:
            response.type = OP_ERROR;
            strcpy(response.payload.error_msg, "Unknown operation");
    }
    send(fd, &response, sizeof(response), 0);
}

void run_epoll_server(int port) {
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0); // создаем TCP сокет
    if (listen_fd < 0) { perror("socket"); return; }

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    set_nonblocking(listen_fd);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr)); 
    addr.sin_family = AF_INET; // указываем семейство адресов
    addr.sin_addr.s_addr = INADDR_ANY; // слушаем на всех интерфейсах
    addr.sin_port = htons(port); // указываем порт для прослушивания

    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) { // привязываем сокет к адресу и порту
        perror("bind");
        return;
    }

    if (listen(listen_fd, 10) < 0) {
        perror("listen");
        return;
    }

    int epoll_fd = epoll_create1(0); // создаем epoll объект событий
    if (epoll_fd < 0) { perror("epoll_create"); return; }

    struct epoll_event ev, events[MAX_EVENTS]; // структура для добавления сокетов в epoll и массив для получения событий
    ev.events = EPOLLIN; // интересуемся событиями чтения
    ev.data.ptr = NULL; // указываем NULL для слушающего сокета
    //ID самого объекта epol, команда, что именно нужно сделать, номер конкретного сокета, какие события ловить
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &ev); // добавляем слушающий сокет в epoll

    printf("Epoll Server listening on port %d...\n", port);

    while (1) {
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1); // ждем событий на сокетах
        for (int i = 0; i < nfds; i++) { // обрабатываем каждое событие
            if (events[i].data.ptr == NULL) { // новый клиент подключился к слушающему сокету
                while (1) {
                    struct sockaddr_in client_addr;
                    socklen_t client_len = sizeof(client_addr);
                    int conn_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_len); // принимаем новое соединение
                    if (conn_fd < 0) {
                        if (errno != EAGAIN && errno != EWOULDBLOCK) perror("accept");
                        break;
                    }
                    set_nonblocking(conn_fd);
                    
                    client_state_t *state = calloc(1, sizeof(client_state_t));
                    state->fd = conn_fd;
                    state->state = STATE_RECV_PACKET;
                    
                    ev.events = EPOLLIN | EPOLLET; // используем режим с блокировкой для новых клиентов
                    ev.data.ptr = state;
                    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_fd, &ev);
                    printf("New client connected: %d\n", conn_fd);
                }
            } else {
                client_state_t *state = (client_state_t *)events[i].data.ptr;
                int fd = state->fd;

                if (state->state == STATE_RECV_PACKET) { // читаем пакет от клиента
                    packet_t pkt;
                    ssize_t n = recv(fd, &pkt, sizeof(pkt), 0);
                    if (n <= 0) {
                        if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) continue;
                        printf("Client disconnected: %d\n", fd);
                        close_client(epoll_fd, state);
                    } else if (n == sizeof(pkt)) {
                        if (pkt.type == OP_FILE_TRANSFER) {
                            char path[300];
                            snprintf(path, sizeof(path), "epoll_uploaded_%s", pkt.payload.file_meta.filename);
                            state->fp = fopen(path, "wb");
                            state->bytes_remaining = pkt.payload.file_meta.size;
                            state->state = STATE_RECV_FILE;
                            printf("Starting file transfer for %d: %s\n", fd, path);
                        } else {
                            handle_math(fd, &pkt);
                        }
                    }
                } else if (state->state == STATE_RECV_FILE) {
                    char buffer[CHUNK_SIZE];
                    while (state->bytes_remaining > 0) {
                        ssize_t n = recv(fd, buffer, sizeof(buffer), 0);
                        if (n <= 0) {
                            if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) break;
                            close_client(epoll_fd, state);
                            break;
                        }
                        fwrite(buffer, 1, n, state->fp);
                        state->bytes_remaining -= n;
                        if (state->bytes_remaining <= 0) {
                            printf("File transfer complete for %d\n", fd);
                            packet_t res;
                            res.type = OP_RESULT;
                            res.payload.result.value = 0;
                            send(fd, &res, sizeof(res), 0);
                            state->state = STATE_RECV_PACKET;
                            fclose(state->fp);
                            state->fp = NULL;
                        }
                    }
                }
            }
        }
    }
}
