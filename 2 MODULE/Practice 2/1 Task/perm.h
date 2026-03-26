#ifndef PERM_H
#define PERM_H

#include <stdio.h>

unsigned perm_parse_octal(const char *s);
int perm_parse_rwx_string(const char *s, unsigned *out_mode);
void perm_print_all(unsigned mode);
int perm_stat_path(const char *path, unsigned *out_mode);
unsigned perm_apply_chmod(unsigned mode, const char *spec);

#endif
