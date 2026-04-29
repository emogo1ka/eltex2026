#ifndef SNIFFER_H
#define SNIFFER_H

#include <stdint.h>
#include <stdio.h>

#define LOG_FILE "sniff_log.txt"

void start_sniffing(int target_port);
void hex_dump(const uint8_t *data, int size, FILE *out);

#endif // SNIFFER_H
