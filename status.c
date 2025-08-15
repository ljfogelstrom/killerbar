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
#include <sys/timerfd.h>
#include <poll.h>

#include "util.h"

#define ARM_TIMER(x) timerfd_settime(x.timerfd, 0, &x.timerspec, NULL)

/* concept: assign each status block to an unique realtime signal */
typedef struct {
    const char                  *(*function)(const char *);
    const char                  *arg;
    const struct itimerspec     timerspec;
    const _Bool                 decay;
    int                         timerfd;
    int                         sigfd;
} Blocks;

#include "helpers.h"
static Blocks block[] = {
    { datetime,     "%F %T",        { .it_interval.tv_sec = 1, .it_value.tv_sec = 1 },   0    },
    { hello_world,     "- %s -",           { .it_interval.tv_sec = 3, .it_value.tv_sec = 1 },    0 }
};

char buffer[512];
static const unsigned int length = LEN(block);
static struct sigaction sigact; // memset(ev, 0, sizeof(struct sigevent)); unneccessary, because global/static structs are zero-initialized
static sigset_t sigset;

static char status[5][256];
static char result[512];
static struct pollfd pfds[5];

void init(void)
{
    for (int i = 0; i<length; i++) {
        if ((block[i].timerfd = timerfd_create(CLOCK_REALTIME, 0)) < 0) // changing the order of assignment seems to fix the bug... could have to do with function arguments being declared 'restrict'?
            err(EXIT_FAILURE, "%d", errno);
        //every block gets a timer
        pfds[i].fd = block[i].timerfd;
        pfds[i].events = POLLIN;
        if (!block[i].decay) { // only non-decaying blocks have timers armed
            if (ARM_TIMER(block[i]) < 0)
                warn("%d", errno);
        }

        if (sigaction(SIGRTMIN + i, &sigact, NULL) < 0)
            warn("%d", errno);
        sigaddset(&sigact.sa_mask, SIGRTMIN + i); // make sure all realtimesignals are blocked during handling
    }
}

void handler (int signo)
{
    int i = signo - SIGRTMIN;
    write(block[i].timerfd, "", 0);
    if (block[i].decay) {
        ARM_TIMER(block[i]);
    }
    write(STDERR_FILENO, "Signal received", 16);

    if (i < 0) {
        fprintf(stderr, "Standard signal received");
    }
}

int main()
{
    Display *dpy = XOpenDisplay(NULL);
    warn(NULL);
    int screen = XDefaultScreen(dpy);
    Window root = RootWindow(dpy, screen);

    int ready = 0;
    uint64_t waste;

    sigfillset(&sigset);

    sigact.sa_handler = handler;
    // sigact.sa_flags = 0; unneccessary?

    init();

    /* install process signal mask
    sigdelset(&sigset, SIGINT);
    sigdelset(&sigset, SIGSEGV);
    sigprocmask(SIG_SETMASK, &sigset, NULL);
    POTENTIALLY CHANGE THIS */ 

    for(;;) {
        ready = poll(pfds, length, -1);
        // printf("pollnr: %d, length: %d\n", ready, length);
        if (ready) for (int i = 0; i<length; i++) {
            if (pfds[i].revents & POLLIN) {
                read(pfds[i].fd, &waste, 8);
                block[i].function(block[i].arg);
                if (strcmp(status[i], buffer))
                    strcpy(status[i], buffer);
            }
        }
        for (int j = length; j >= 0; j--) {
            strcat(result, status[j]);
        }

        puts(result);
        XStoreName(dpy, root, result);
        XFlush(dpy);
        memset(result, 0, 256);
    }// wait

    XCloseDisplay(dpy);

    return 0;
}
