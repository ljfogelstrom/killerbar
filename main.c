/* TODO:
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
#include "status.h"

#define ERR(x)          err(EXIT_FAILURE, # x": %d", errno);
#define TIMER(x, y)     { { (int)x, (long)((x - (int)x) * 10e8) }, \
                          { (int)y, (long)((y - (int)y) * 10e8) } }

/* concept: assign each status block to an unique realtime signal */
typedef struct {
    const char                  *(*function)(const char *);
    const char                  *arg;
    const char                  *fmt;
    struct itimerspec           timerspec;
    struct sigevent             event;
    timer_t                     timer;
} Blocks;

static Blocks block[] = {
    /* function         argument                            format          interval / initial timer value      */
    { datetime,         "%F %T",                "| %s",                             TIMER(1, 1)         },
    { run_command,      "setvolume",            "| %s",                             TIMER(0, 3)         },
    { temp,      "/sys/class/thermal/thermal_zone2/temp",   "%sC",                  TIMER(10, 1)        },
    { cpu_perc,         NULL,                               "%s",                   TIMER(10, 3)        },
};

/* set this if you don't want to mess around with the formatting too much */
static char* delimiter = " ";

static constexpr unsigned int length = LEN(block);
static sigset_t sigset;

char buffer[512];
static char status[length][256];
static char result[512];

static volatile int running = 1;

static _Bool sflag, vflag;


static void init(void)
{
    int i, ret;
    for (i = 0; i<length; i++) { /* initialize continuous blocks */
        if (block[i].timerspec.it_interval.tv_sec || block[i].timerspec.it_interval.tv_nsec ) { /* yep */
            if (!memcmp(&block[i].timerspec.it_value, &(time_t){ 0 }, sizeof(time_t)))
                block[i].timerspec.it_value.tv_sec = 1;
                /* default initial timer value */
        block[i].event.sigev_notify = SIGEV_SIGNAL;
        block[i].event.sigev_signo = SIGRTMIN + i;
        ret = timer_create(CLOCK_REALTIME, &block[i].event, &block[i].timer);
        if (ret) ERR(timer_create);
        ret = timer_settime(block[i].timer, 0, &block[i].timerspec, NULL);
        if (ret) ERR(timer_settime);
        } else { /* initialize intermittent blocks */
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
    /* hides the block when timer expires */
}

void handle_signal_std (int signo)
{
    switch (signo) {
        case SIGCHLD:
            if (vflag) fprintf(stderr, "SIGCHLD received");
            break;
        default:
            running = 0;
            break;
    }
}

int main(int argc, char *argv[])
{
    short opt;
    while ((opt = getopt(argc, argv, "sv")) != -1) {
        switch (opt) {
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
    while (running) {
        sigwait(&sigset, &signo);
        if (vflag) fprintf(stderr, "Signal received: %d\n", signo);
        int i = signo - SIGRTMIN;
        if (i < 0)
            handle_signal_std(signo);
        else if (i > length)
            fprintf(stderr, "Signal %d is unassigned\n", signo);
        else {
            block[i].function(block[i].arg);
            if (!block[i].timerspec.it_interval.tv_sec && !block[i].timerspec.it_interval.tv_nsec) { /* is intermittent */
                timer_settime(block[i].timer, 0, &block[i].timerspec, NULL);
            }
            sprintf(status[i], block[i].fmt, buffer);
            for (int j = length-1; j >= 0; j--) {
                strcat(result, status[j]);
                if (j && *status[j]) strcat(result, delimiter);
            }
            if (sflag || vflag) puts(result);
            else {
                XStoreName(dpy, root, result);
                XFlush(dpy);
            }
            memset(result, 0, 256);
            memset(buffer, 0, 512); /* added this additional call so that the same
                                     * buffer contents are not printed repeated times by accident */
        }
    }

    fprintf(stderr, "Signal received: %d\n", signo);

    XCloseDisplay(dpy);

    return 0;
}
