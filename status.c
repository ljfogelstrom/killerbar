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
#include <sys/signalfd.h>
#include <poll.h>
#include <fcntl.h>
#include <pthread.h>

#include "util.h"
#include "helpers.h"

#define ARM_TIMER(x) timerfd_settime(x.timerfd, 0, &x.timerspec, NULL)

/* concept: assign each status block to an unique realtime signal */
typedef struct {
    const char                  *(*function)(const char *);
    const char                  *arg;
    const struct itimerspec     timerspec;
    const _Bool                 decaying;
    _Bool                       visible;
    int                         timerfd;
    int                         sigfd;
} Blocks;

static Blocks block[] = {
    { datetime,     "%F %T",        { .it_interval.tv_sec = 1, .it_value.tv_sec = 1 },                  0, 1 },
    { hello_world,     "- %s - ",           { .it_interval.tv_sec = 3, .it_value.tv_sec = 1 },          0, 1 },
    { hello_world,       "echo %s 'hello'",      { .it_interval.tv_sec = 0, .it_value.tv_sec = 3 },  1, 0 },
};

#define BLOCKS 3

char buffer[512];
static const unsigned int length = LEN(block);
static struct sigaction sigact;

static char status[BLOCKS][256];
static char result[512];
static struct pollfd pfds[2][BLOCKS];

volatile sig_atomic_t finished = 0;
static _Bool oneflag=0, sflag=0, vflag=0; /* can make a bitfield here */

enum { TIMERS, SIGNALS };


static void init(void)
{
    sigset_t mask;

    for (int i = 0; i<length; i++) {
        /* initialize timers */
        if ((block[i].timerfd = timerfd_create(CLOCK_REALTIME, 0)) < 0)
            err(EXIT_FAILURE, "timerfd_create: %d", errno);
        pfds[TIMERS][i].fd = block[i].timerfd;
        pfds[TIMERS][i].events = POLLIN;
        if (!block[i].decaying) { /* TODO */
            if (ARM_TIMER(block[i]) < 0)
                warn("ARM_TIMER: %d", errno);
        }
        /* initialize signals */
        sigemptyset(&mask);
        sigaddset(&mask, SIGRTMIN + i);
        block[i].sigfd = signalfd(-1, &mask, 0);
        pfds[SIGNALS][i].fd = block[i].sigfd;
        pfds[SIGNALS][i].events = POLLIN;
    }
}

void handler (int signo)
{
    int n = signo - SIGRTMIN;
    if (vflag) {
        char signr[21] = "Signal received: "; sigtoa(signo, signr+17);
        write(STDERR_FILENO, signr, 21);
    }
    if (n < 0) { /* standard signal received */
        finished = 1;
        return;
    }
}

#if 0
void signal_loop (void)
{
    int ready;
    struct signalfd_siginfo siginfo; // hate this (or do i...)

    printf("hello");
    while (!finished) {
        ready = poll(pfds[SIGNALS], BLOCKS, -1);
        if (ready) for (int i = 0; i<BLOCKS; i++) {
            if (pfds[SIGNALS][i].revents & POLLIN) {
                read(pfds[SIGNALS][i].fd, &siginfo, sizeof(struct signalfd_siginfo)); /* could be useful later */
                block[i].function(block[i].arg);
                if (strcmp(status[i], buffer))
                    strcpy(status[i], buffer);
            }
        }
        for (int j = length; j >= 0; j--) {
            strcat(result, status[j]);
        }
    }
}
#endif /* might implement in pthreads */

int main(int argc, char *argv[])
{
    if (LEN(block) != BLOCKS) {
        fprintf(stderr, "Runtime error: 'BLOCKS (%d)' not equal to number of declared blocks (%d)\n", BLOCKS, length); 
        exit(EXIT_FAILURE);
    }

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
    
    sigact.sa_handler = handler;
    for (int i = 1; i < 31; i++) {
        if (sigaction(i, &sigact, NULL) < 0 && vflag) /* certain signals will be ignored by default (0, 9, SIGSTOP, 32, 33) */
            warn("sigaction: (signal = %d) %d", i, errno);
    }

    /* install signal mask */
    sigset_t sigset;
    sigemptyset(&sigset);
    for (int i = SIGRTMIN; i < SIGRTMAX; i++) {
        sigaddset(&sigset, i);
    }
    sigprocmask(SIG_SETMASK, &sigset, NULL);

    int ready = 0;
    uint64_t dummy;
    struct signalfd_siginfo siginfo; // hate this

    /* main event loop */
    while (!finished) {
        ready = poll(pfds[TIMERS], BLOCKS, -1); /* could maybe try a short block to save on the logic */
        if (ready) for (int i = 0; i<BLOCKS; i++) {
            if (pfds[TIMERS][i].revents & POLLIN) {
                if (block[i].decaying) {
                    block[i].visible = 0;
                }
                read(pfds[TIMERS][i].fd, &dummy, 8); /* could be useful later */
                block[i].function(block[i].arg);
                if (strcmp(status[i], buffer))
                    strcpy(status[i], buffer);
            }
        }
        ready = poll(pfds[SIGNALS], BLOCKS, 1); /* TODO: exec in new thread */
        if (ready) for (int i = 0; i<BLOCKS; i++) {
            if (pfds[SIGNALS][i].revents & POLLIN) {
                read(pfds[SIGNALS][i].fd, &siginfo, sizeof(struct signalfd_siginfo)); /* could be useful later */
                if (block[i].decaying) ARM_TIMER(block[i]);
                block[i].visible = 1;
                block[i].function(block[i].arg);
                if (strcmp(status[i], buffer))
                    strcpy(status[i], buffer);
            }
        }
        for (int j = length; j >= 0; j--) {
            if (block[j].visible)
                strcat(result, status[j]);
        }

        if (sflag || vflag) puts(result);
        else {
            XStoreName(dpy, root, result);
            XFlush(dpy);
        }
        memset(result, 0, 256);
    }

    XCloseDisplay(dpy);

    return 0;
}
