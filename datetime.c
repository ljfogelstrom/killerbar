/* See LICENSE file for copyright and license details. */
#include <stdio.h>
#include <time.h>

#include "util.h"
#include "helpers.h"

extern char buffer[];

const char *
datetime(const char *fmt)
{
	time_t t;
	t = time(NULL);
	if (!strftime(buffer, sizeof(buf), fmt, localtime(&t))) {
		warn("strftime: Result string exceeds buffer size");
		return NULL;
	}

	return buffer;
}
