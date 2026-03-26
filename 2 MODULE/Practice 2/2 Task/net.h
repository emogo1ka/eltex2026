#ifndef NET_H
#define NET_H

#include <stdint.h>

int net_parse_ipv4(const char *s, uint32_t *out);
void net_format_ipv4(uint32_t addr, char buf[16]);
int net_same_subnet(uint32_t gw, uint32_t mask, uint32_t dest);
uint32_t net_random_ip(void);

#endif
