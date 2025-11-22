/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <string.h>

#include "status.h"
#include "util.h"

const char *
cat(const char *path)
{
        char *f;
        FILE *fp;

        if (!(fp = fopen(path, "r"))) {
                warn("fopen '%s':", path);
                return NULL;
        }

        f = fgets(buffer, sizeof(buffer) - 1, fp);
        if (fclose(fp) < 0) {
                warn("fclose '%s':", path);
                return NULL;
        }
        if (!f)
                return NULL;

        if ((f = strrchr(buffer, '\n')))
                f[0] = '\0';

        return buffer[0] ? buffer : NULL;
}

