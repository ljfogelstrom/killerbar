/* TODO:
 *      - write a macro for assigning timer values and intervals
 *      - add logic for 'decaying' status blocks
 *      - (in the future) add multi-threading and wide-string support
 */
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <err.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <X11/Xlib.h>

#include <features.h>
#define _ISOC23_SOURCE

#include "util.h"
#include "helpers.h"

#define ARM_TIMER(x)    timer_settime(x.timer, 0, &x.timerspec, NULL)
#define ERR(x)          err(EXIT_FAILURE, # x": %d", errno);
/* concept: assign each status block to an unique realtime signal */
typedef struct {
    const char                  *(*function)(const char *);
    const char                  *arg;
    const struct itimerspec     timerspec;
    const _Bool                 decay;
    struct sigevent             event;
    timer_t                     timer;
} Blocks;

static Blocks block[] = {
    { datetime,     "%F %T",        { .it_interval.tv_sec = 1, .it_value.tv_sec = 1 },                  0 },
    { hello_world,     "- %s - ",           { .it_interval.tv_sec = 3, .it_value.tv_sec = 1 },          0 },
    { hello_world,       "echo %s 'hello'",      { .it_interval.tv_sec = 0,    .it_value.tv_sec = 3 },  1 },
};

static constexpr unsigned int length = LEN(block);
static sigset_t sigset;

char buffer[512];
static char status[length][256];
static char result[512];

static _Bool oneflag=0, sflag=0, vflag=0;

void notify (union sigval arg);

static void init(void)
{
    int i, ret;
    for (i = 0; i<length; i++) { /* just give em all a signal */
        if (block[i].timerspec.it_interval.tv_sec || block[i].timerspec.it_interval.tv_nsec ) { /* yep */
        block[i].event.sigev_notify = SIGEV_SIGNAL;
        block[i].event.sigev_signo = SIGRTMIN + i;
        ret = timer_create(CLOCK_REALTIME, &block[i].event, &block[i].timer);
        if (ret) ERR(timer_create);
        ret = ARM_TIMER(block[i]);
        if (ret) ERR(timer_settime);
        } else {
            block[i].event.sigev_notify = SIGEV_THREAD;
            block[i].event.sigev_notify_function = notify;
            block[i].event.sigev_value.sival_int = i;
            ret = timer_create(CLOCK_REALTIME, &block[i].event, &block[i].timer);
            if (ret) ERR(timer_create);
        }
    }
}

void notify (union sigval arg)
{
    int i = arg.sival_int;
    strcpy(status[i], "");
}

int main(int argc, char *argv[])
{
    short opt;
    while ((opt = getopt(argc, argv, "1sv")) != -1) {
        switch (opt) {
            case '1': // run all commands once
                oneflag = 1;
                break;
            case 's': // print to stdout
                sflag = 1;
                break;
            case 'v': // verbose output; implies s
                vflag = 1;
                break;
        }
    }

    Display *dpy = XOpenDisplay(NULL);
    int screen = XDefaultScreen(dpy);
    Window root = RootWindow(dpy, screen); /* these functions will always set errno for some reason */

    init();

    /* block all signals */
    sigfillset(&sigset);
    sigprocmask(SIG_SETMASK, &sigset, NULL);

    int signo;
    /* main event loop */
    while (1) {
        sigwait(&sigset, &signo);
        if (vflag) fprintf(stderr, "Signal received: %d\n", signo);
        int i = signo - SIGRTMIN;
        if (i < 0)
            break;
        block[i].function(block[i].arg);
        if (block[i].decay) {
            ARM_TIMER(block[i]);
        }
        strcpy(status[i], buffer);
        for (int j = length-1; j >= 0; j--) {
            strcat(result, status[j]);
        }
        if (sflag || vflag) puts(result);
        else {
            XStoreName(dpy, root, result);
            XFlush(dpy);
        }
        memset(result, 0, 256);
    }

    fprintf(stderr, "Signal received: %d\n", signo);

    XCloseDisplay(dpy);

    return 0;
}
