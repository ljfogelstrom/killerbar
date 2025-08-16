/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <string.h>

#include "util.h"
#include "helpers.h"

const char *
run_command(const char *cmd)
{
	char *p;
	FILE *fp;

	if (!(fp = popen(cmd, "r"))) {
		warn("popen '%s':", cmd);
		return NULL;
	}

	p = fgets(buffer, sizeof(buffer) - 1, fp);
	if (pclose(fp) < 0) {
		warn("pclose '%s':", cmd);
		return NULL;
	}
	if (!p)
		return NULL;

	if ((p = strrchr(buffer, '\n')))
		p[0] = '\0';

	return buffer[0] ? buffer : NULL;
}
