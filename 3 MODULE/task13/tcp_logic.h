#ifndef TCP_LOGIC_H
#define TCP_LOGIC_H

#include "protocol.h"

int setup_server(int port);
int connect_to_server(const char *ip, int port);
void handle_client(int client_fd);
void send_math_request(int sockfd, op_type_t type, double a, double b);
void send_file(int sockfd, const char *filepath);

#endif // TCP_LOGIC_H
