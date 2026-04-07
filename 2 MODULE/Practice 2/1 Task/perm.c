#include "perm.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
#include <sys/types.h>
#include <sys/stat.h>
#else
#include <sys/stat.h>
#endif

#define S_PERM (S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH)

static int is_oct_digit(char c) { return c >= '0' && c <= '7'; }

static unsigned rwx_one(char c, unsigned br, unsigned bw, unsigned bx) {
    if (c == 'r')
        return br;
    if (c == 'w')
        return bw;
    if (c == 'x')
        return bx;
    if (c == '-')
        return 0;
    return (unsigned)-1;
}

int perm_parse_rwx_string(const char *s, unsigned *out_mode) {
    const char *p = s;
    while (*p == ' ' || *p == '\t')
        p++;
    if (*p == '-' && (p[1] == 'r' || p[1] == 'w' || p[1] == '-'))
        p++;
    if (strlen(p) < 9)
        return -1;
    unsigned mode = 0;
    for (int g = 0; g < 3; g++) {
        unsigned br = (g == 0) ? S_IRUSR : (g == 1) ? S_IRGRP : S_IROTH;
        unsigned bw = (g == 0) ? S_IWUSR : (g == 1) ? S_IWGRP : S_IWOTH;
        unsigned bx = (g == 0) ? S_IXUSR : (g == 1) ? S_IXGRP : S_IXOTH;
        int o = g * 3;
        unsigned a = rwx_one(p[o + 0], br, bw, bx);
        unsigned b = rwx_one(p[o + 1], br, bw, bx);
        unsigned c = rwx_one(p[o + 2], br, bw, bx);
        if (a == (unsigned)-1 || b == (unsigned)-1 || c == (unsigned)-1)
            return -1;
        mode |= a | b | c;
    }
    *out_mode = mode;
    return 0;
}

unsigned perm_parse_octal(const char *s) {
    const char *p = s;
    while (*p == ' ' || *p == '\t')
        p++;
    unsigned v = 0;
    int digits = 0;
    while (is_oct_digit(*p)) {
        v = (v << 3) | (unsigned)(*p - '0');
        p++;
        digits++;
        if (digits > 4)
            break;
    }
    return v & 07777U;
}

static void mode_to_rwx(unsigned mode, char out[10]) {
    out[0] = (mode & S_IRUSR) ? 'r' : '-';
    out[1] = (mode & S_IWUSR) ? 'w' : '-';
    out[2] = (mode & S_IXUSR) ? 'x' : '-';
    out[3] = (mode & S_IRGRP) ? 'r' : '-';
    out[4] = (mode & S_IWGRP) ? 'w' : '-';
    out[5] = (mode & S_IXGRP) ? 'x' : '-';
    out[6] = (mode & S_IROTH) ? 'r' : '-';
    out[7] = (mode & S_IWOTH) ? 'w' : '-';
    out[8] = (mode & S_IXOTH) ? 'x' : '-';
    out[9] = '\0';
}

static void print_bits(unsigned mode) {
    unsigned m = mode & 0777U;
    char bits[10];
    int i = 0;
    for (int b = 8; b >= 0; b--)
        bits[i++] = (m & (1u << b)) ? '1' : '0';
    bits[i] = '\0';
    printf("биты (rwx по порядку): %s\n", bits);
}

static unsigned mode_to_octal_display(unsigned mode) {
    return ((mode & S_IRUSR ? 4U : 0U) | (mode & S_IWUSR ? 2U : 0U) | (mode & S_IXUSR ? 1U : 0U)) * 64U +
           ((mode & S_IRGRP ? 4U : 0U) | (mode & S_IWGRP ? 2U : 0U) | (mode & S_IXGRP ? 1U : 0U)) * 8U +
           ((mode & S_IROTH ? 4U : 0U) | (mode & S_IWOTH ? 2U : 0U) | (mode & S_IXOTH ? 1U : 0U));
}

void perm_print_all(unsigned mode) {
    char rwx[10];
    mode_to_rwx(mode & S_PERM, rwx);
    printf("буквенно: %s\n", rwx);
    printf("цифрово (восьмеричное): %03o\n", (unsigned)mode_to_octal_display(mode));
    print_bits(mode);
}

int perm_stat_path(const char *path, unsigned *out_mode) {
    struct stat st;
    if (stat(path, &st) != 0) {
        perror("stat");
        return -1;
    }
    *out_mode = (unsigned)(st.st_mode & 07777U);
    return 0;
}

static int clause_all_octal(const char *clause) {
    const char *p = clause;
    while (*p == ' ' || *p == '\t')
        p++;
    if (*p == '\0')
        return 0;
    for (; *p; p++) {
        if (*p == ' ' || *p == '\t')
            continue;
        if (!is_oct_digit(*p))
            return 0;
    }
    return 1;
}

static unsigned who_mask_from_str(const char *who) {
    unsigned m = 0;
    for (int i = 0; who[i]; i++) {
        if (who[i] == 'u')
            m |= S_IRUSR | S_IWUSR | S_IXUSR;
        else if (who[i] == 'g')
            m |= S_IRGRP | S_IWGRP | S_IXGRP;
        else if (who[i] == 'o')
            m |= S_IROTH | S_IWOTH | S_IXOTH;
        else if (who[i] == 'a')
            m |= S_PERM;
    }
    if (m == 0)
        m = S_PERM;
    return m & 0777U;
}

static unsigned perm_mask_from_str(const char *p) {
    unsigned pm = 0;
    for (; *p && *p != ','; p++) {
        if (*p == ' ' || *p == '\t')
            continue;
        if (*p == 'r')
            pm |= S_IRUSR | S_IRGRP | S_IROTH;
        else if (*p == 'w')
            pm |= S_IWUSR | S_IWGRP | S_IWOTH;
        else if (*p == 'x')
            pm |= S_IXUSR | S_IXGRP | S_IXOTH;
    }
    return pm & 0777U;
}

static unsigned apply_one_clause(unsigned mode, const char *clause) {
    const char *p = clause;
    while (*p == ' ' || *p == '\t')
        p++;
    if (*p == '\0')
        return mode;

    if (clause_all_octal(clause)) {
        while (*p == ' ' || *p == '\t')
            p++;
        return perm_parse_octal(p) & 0777U;
    }

    char who[8];
    int nw = 0;
    while (*p == 'u' || *p == 'g' || *p == 'o' || *p == 'a') {
        if (nw < 7)
            who[nw++] = *p;
        p++;
    }
    who[nw] = '\0';
    while (*p == ' ' || *p == '\t')
        p++;
    if (nw == 0 && (*p == '+' || *p == '-' || *p == '=')) {
        who[nw++] = 'a';
        who[nw] = '\0';
    }

    char op = *p++;
    if (op != '+' && op != '-' && op != '=')
        return mode;
    while (*p == ' ' || *p == '\t')
        p++;

    if (who[0] == '\0')
        return mode;

    unsigned wm = who_mask_from_str(who); // u/g/o
    unsigned pm = perm_mask_from_str(p); //r/w/x
    unsigned mp = pm & wm; //пересечение, какие биты менять
    if (op == '=') {
        mode = (mode & ~wm) | mp;
    } else if (op == '+') {
        mode = mode | mp;
    } else if (op == '-') {
        mode = mode & ~mp;
    }
    return mode;
}

unsigned perm_apply_chmod(unsigned mode, const char *spec) {
    unsigned m = mode & 0777U;
    const char *p = spec;
    while (p && *p) {
        const char *end = strchr(p, ',');
        if (!end)
            end = p + strlen(p);
        char buf[256];
        size_t len = (size_t)(end - p);
        if (len >= sizeof(buf))
            len = sizeof(buf) - 1;
        memcpy(buf, p, len);
        buf[len] = '\0';
        m = apply_one_clause(m, buf);
        m &= 0777U;
        if (*end == '\0')
            break;
        p = end + 1;
    }
    return m;
}
