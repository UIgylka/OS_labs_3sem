#include <unistd.h>
#include "contracts.h"

extern float sin_integral(float a, float b, float e);
extern int gcd(int a, int b);

static void print_str(const char *s) {
    int n = 0;
    while (s[n]) n++;
    write(STDOUT_FILENO, s, n);
}

static void print_int(int v) {
    char buf[32];
    int i = 0;
    if (v == 0) { write(STDOUT_FILENO, "0\n", 2); return; }
    int neg = 0;
    if (v < 0) { neg = 1; v = -v; }
    while (v > 0) { buf[i++] = '0' + (v % 10); v /= 10; }
    if (neg) buf[i++] = '-';
    for (int j = 0; j < i / 2; j++) { char t = buf[j]; buf[j] = buf[i-j-1]; buf[i-j-1] = t; }
    write(STDOUT_FILENO, buf, i);
    write(STDOUT_FILENO, "\n", 1);
}

static void print_float(float v) {
    int neg = 0;
    if (v < 0) { neg = 1; v = -v; }
    int ip = (int)v;
    float fp = v - (float)ip;
    if (neg) write(STDOUT_FILENO, "-", 1);
    print_int(ip);
    char buf[16];
    int i = 0;
    buf[i++] = '.';
    for (int k = 0; k < 6; k++) {
        fp *= 10.0f;
        int d = (int)fp;
        buf[i++] = '0' + d;
        fp -= (float)d;
    }
    write(STDOUT_FILENO, buf, i);
    write(STDOUT_FILENO, "\n", 1);
}

static const char *skip_ws(const char *s) {
    while (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r') s++;
    return s;
}

static const char *parse_int(const char *s, int *out) {
    s = skip_ws(s);
    int sign = 1;
    if (*s == '-') { sign = -1; s++; }
    else if (*s == '+') s++;
    int val = 0;
    while (*s >= '0' && *s <= '9') { val = val * 10 + (*s - '0'); s++; }
    *out = val * sign;
    return s;
}

static const char *parse_float(const char *s, float *out) {
    s = skip_ws(s);
    int sign = 1;
    if (*s == '-') { sign = -1; s++; }
    else if (*s == '+') s++;
    float val = 0.0f;
    while (*s >= '0' && *s <= '9') { val = val * 10.0f + (float)(*s - '0'); s++; }
    if (*s == '.') {
        s++;
        float frac = 0.1f;
        while (*s >= '0' && *s <= '9') { val += (float)(*s - '0') * frac; frac *= 0.1f; s++; }
    }
    if (*s == 'e' || *s == 'E') {
        s++;
        int esign = 1;
        if (*s == '-') { esign = -1; s++; }
        else if (*s == '+') s++;
        int exp = 0;
        while (*s >= '0' && *s <= '9') { exp = exp * 10 + (*s - '0'); s++; }
        float mul = 1.0f;
        for (int i = 0; i < exp; i++) mul *= 10.0f;
        if (esign < 0) val /= mul; else val *= mul;
    }
    *out = val * sign;
    return s;
}

int main(void) {
    char buf[1024];
    print_str("Program 1 (compile-time linking, impl #1)\n");
    print_str("Commands: '1 a b e' | '2 a b'\n");
    while (1) {
        int n = read(STDIN_FILENO, buf, sizeof(buf) - 1);
        if (n <= 0) break;
        buf[n] = '\0';
        const char *p = skip_ws(buf);
        if (*p == '1') {
            float a, b, e;
            p = parse_float(p + 1, &a);
            p = parse_float(p, &b);
            p = parse_float(p, &e);
            print_str("Integral = ");
            print_float(sin_integral(a, b, e));
        } else if (*p == '2') {
            int a, b;
            p = parse_int(p + 1, &a);
            p = parse_int(p, &b);
            print_str("GCD = ");
            print_int(gcd(a, b));
        } else {
            print_str("Unknown command\n");
        }
    }
    return 0;
}