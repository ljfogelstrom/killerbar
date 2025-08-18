#include "util.h"

void sigtoa (int n, char* ret)
{
    ret[3] = 0;
    ret[2] = 0x0a;
    ret[1] = (n % 10) + 48;
    ret[0] = (n / 10) + 48;
}
