#include <stdio.h>
#include "util.h"
#include "helpers.h"

extern char buffer[];

const char* hello_world (const char* fmt) {
    sprintf(buffer, fmt, "hello world");
    return buffer;
}
