/* See LICENSE file for copyright and license details. */
#include <stdint.h>
#include <stddef.h>

extern char buffer[512];

#define LEN(x) (sizeof(x) / sizeof((x)[0]))

extern char *argv0;

void warn(const char *, ...);
void die(const char *, ...);

int esnprintf(char *str, size_t size, const char *fmt, ...);
const char *bprintf(const char *fmt, ...);
const char *fmt_human(uintmax_t num, int base);
int pscanf(const char *path, const char *fmt, ...);
void sigtoa(int n, char* ret);
