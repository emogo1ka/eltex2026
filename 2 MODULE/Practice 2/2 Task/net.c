#include "net.h"
#include <stdio.h>
#include <stdlib.h>

int net_parse_ipv4(const char *s, uint32_t *out) {
    unsigned a = 0, b = 0, c = 0, d = 0;
    if (sscanf(s, "%u.%u.%u.%u", &a, &b, &c, &d) != 4)
        return -1;
    if (a > 255U || b > 255U || c > 255U || d > 255U)
        return -1;
    *out = ((uint32_t)a << 24) | ((uint32_t)b << 16) | ((uint32_t)c << 8) | (uint32_t)d;
    return 0;
}

void net_format_ipv4(uint32_t addr, char buf[16]) {
    unsigned a = (unsigned)((addr >> 24) & 0xFFU);
    unsigned b = (unsigned)((addr >> 16) & 0xFFU);
    unsigned c = (unsigned)((addr >> 8) & 0xFFU);
    unsigned d = (unsigned)(addr & 0xFFU);
    snprintf(buf, 16, "%u.%u.%u.%u", a, b, c, d);
}

int net_same_subnet(uint32_t gw, uint32_t mask, uint32_t dest) {
    return (gw & mask) == (dest & mask);
}

uint32_t net_random_ip(void) {
    return ((uint32_t)(rand() & 0xFF) << 24) | ((uint32_t)(rand() & 0xFF) << 16) |
           ((uint32_t)(rand() & 0xFF) << 8) | (uint32_t)(rand() & 0xFF);
}
