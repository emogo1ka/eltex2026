#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>

#define DEFAULT_PORT 8080
#define BUFFER_SIZE 1024
#define MAX_EVENTS 64

// Протокол
typedef enum {
    OP_EXIT = 0,
    OP_MATH = 1,
    OP_FILE = 2
} command_t;

typedef enum {
    STATE_WAIT_CMD,
    STATE_WAIT_MATH_OP,
    STATE_WAIT_MATH_A,
    STATE_WAIT_MATH_B,
    STATE_WAIT_FILENAME,
    STATE_SENDING_FILE
} client_state_t;

typedef struct {
    int fd;
    client_state_t state;
    char math_op;
    double val_a;
    FILE *file_ptr;
    long file_remaining;
} client_context_t;

void set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void close_client(int epoll_fd, client_context_t *ctx) {
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, ctx->fd, NULL);
    if (ctx->file_ptr) fclose(ctx->file_ptr);
    close(ctx->fd);
    free(ctx);
    printf("[Server] Client disconnected.\n");
}

int main(int argc, char *argv[]) {
    int port = (argc < 2) ? DEFAULT_PORT : atoi(argv[1]);
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) { perror("socket"); return 1; }
    
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind"); return 1;
    }
    listen(listen_fd, 10);
    set_nonblocking(listen_fd);

    int epoll_fd = epoll_create1(0);
    struct epoll_event ev, events[MAX_EVENTS];
    ev.events = EPOLLIN;
    ev.data.ptr = NULL; 
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &ev);

    printf("Multiplexed Server (epoll) started on port %d\n", port);

    while (1) {
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (nfds < 0) {
            if (errno == EINTR) continue;
            perror("epoll_wait"); break;
        }

        for (int i = 0; i < nfds; i++) {
            if (events[i].data.ptr == NULL) {
                // New connection
                struct sockaddr_in cli_addr;
                socklen_t clilen = sizeof(cli_addr);
                int conn_fd = accept(listen_fd, (struct sockaddr *)&cli_addr, &clilen);
                if (conn_fd < 0) continue;
                
                set_nonblocking(conn_fd);
                client_context_t *ctx = calloc(1, sizeof(client_context_t));
                ctx->fd = conn_fd;
                ctx->state = STATE_WAIT_CMD;
                
                struct epoll_event ev_cli;
                ev_cli.events = EPOLLIN | EPOLLET;
                ev_cli.data.ptr = ctx;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_fd, &ev_cli);
                printf("[Server] + Connection from %s\n", inet_ntoa(cli_addr.sin_addr));
            } else {
                client_context_t *ctx = (client_context_t *)events[i].data.ptr;
                
                if (events[i].events & (EPOLLERR | EPOLLHUP)) {
                    close_client(epoll_fd, ctx);
                    continue;
                }

                if (ctx->state == STATE_SENDING_FILE && (events[i].events & EPOLLOUT)) {
                    char file_buf[BUFFER_SIZE];
                    size_t to_read = (ctx->file_remaining < BUFFER_SIZE) ? ctx->file_remaining : BUFFER_SIZE;
                    int n = fread(file_buf, 1, to_read, ctx->file_ptr);
                    if (n > 0) {
                        int sent = write(ctx->fd, file_buf, n);
                        if (sent > 0) ctx->file_remaining -= sent;
                        if (ctx->file_remaining <= 0) {
                            fclose(ctx->file_ptr); ctx->file_ptr = NULL;
                            ctx->state = STATE_WAIT_CMD;
                            struct epoll_event ev_mod;
                            ev_mod.events = EPOLLIN | EPOLLET;
                            ev_mod.data.ptr = ctx;
                            epoll_ctl(epoll_fd, EPOLL_CTL_MOD, ctx->fd, &ev_mod);
                        }
                    }
                    continue;
                }

                if (events[i].events & EPOLLIN) {
                    if (ctx->state == STATE_WAIT_CMD) {
                        int cmd;
                        if (read(ctx->fd, &cmd, sizeof(cmd)) <= 0) { close_client(epoll_fd, ctx); continue; }
                        if (cmd == OP_EXIT) { close_client(epoll_fd, ctx); continue; }
                        ctx->state = (cmd == OP_MATH) ? STATE_WAIT_MATH_OP : STATE_WAIT_FILENAME;
                    } 
                    else if (ctx->state == STATE_WAIT_MATH_OP) {
                        if (read(ctx->fd, &ctx->math_op, 1) <= 0) continue;
                        ctx->state = STATE_WAIT_MATH_A;
                    }
                    else if (ctx->state == STATE_WAIT_MATH_A) {
                        char val[64]; 
                        if (read(ctx->fd, val, 64) <= 0) continue;
                        ctx->val_a = atof(val);
                        ctx->state = STATE_WAIT_MATH_B;
                    }
                    else if (ctx->state == STATE_WAIT_MATH_B) {
                        char val[64]; 
                        if (read(ctx->fd, val, 64) <= 0) continue;
                        double b = atof(val);
                        double res = 0;
                        switch(ctx->math_op) {
                            case '+': res = ctx->val_a + b; break;
                            case '-': res = ctx->val_a - b; break;
                            case '*': res = ctx->val_a * b; break;
                            case '/': res = (b != 0) ? ctx->val_a / b : 0; break;
                        }
                        printf("[Server] Math: %.2f %c %.2f = %.2f\n", ctx->val_a, ctx->math_op, b, res);
                        char resp[BUFFER_SIZE];
                        snprintf(resp, sizeof(resp), "Result: %.2f (Multiplexed)\n", res);
                        write(ctx->fd, resp, strlen(resp) + 1);
                        ctx->state = STATE_WAIT_CMD;
                    }
                    else if (ctx->state == STATE_WAIT_FILENAME) {
                        char filename[256];
                        memset(filename, 0, 256);
                        int n = read(ctx->fd, filename, 256);
                        if (n <= 0) { close_client(epoll_fd, ctx); continue; }
                        
                        // Очистка имени файла от \n или \r
                        filename[strcspn(filename, "\r\n")] = 0;
                        
                        printf("[Server] Opening file: '%s'\n", filename);
                        ctx->file_ptr = fopen(filename, "rb");
                        if (!ctx->file_ptr) {
                            long err = -1; write(ctx->fd, &err, sizeof(err));
                            ctx->state = STATE_WAIT_CMD;
                        } else {
                            fseek(ctx->file_ptr, 0, SEEK_END);
                            ctx->file_remaining = ftell(ctx->file_ptr);
                            rewind(ctx->file_ptr);
                            write(ctx->fd, &ctx->file_remaining, sizeof(long));
                            ctx->state = STATE_SENDING_FILE;
                            struct epoll_event ev_mod;
                            ev_mod.events = EPOLLIN | EPOLLOUT | EPOLLET;
                            ev_mod.data.ptr = ctx;
                            epoll_ctl(epoll_fd, EPOLL_CTL_MOD, ctx->fd, &ev_mod);
                        }
                    }
                }
            }
        }
    }
    return 0;
}
