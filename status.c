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
#include "helpers.h"

#define ARM_TIMER(x) timerfd_settime(x.timerfd, 0, &x.timerspec, NULL)

/* concept: assign each status block to an unique realtime signal */
typedef struct {
    const char                  *(*function)(const char *);
    const char                  *arg;
    const struct itimerspec     timerspec;
    const _Bool                 decay;
    int                         timerfd;
} Blocks;

static Blocks block[] = {
    { datetime,     "%F %T",        { .it_interval.tv_sec = 1, .it_value.tv_sec = 1 },   0    },
    { hello_world,     "- %s - ",           { .it_interval.tv_sec = 3, .it_value.tv_sec = 1 },    0 },
};

#define BLOCKS 2

char buffer[512];
static const unsigned int length = LEN(block);
static struct sigaction sigact;
static sigset_t sigset;

static char status[BLOCKS][256];
static char result[512];
static struct pollfd pfds[BLOCKS];

volatile sig_atomic_t finished = 0;
static int oneflag=0, sflag=0, vflag=0;


static void init(void)
{
    for (int i = 0; i<length; i++) {
        if ((block[i].timerfd = timerfd_create(CLOCK_REALTIME, 0)) < 0)
            err(EXIT_FAILURE, "timerfd_create: %d", errno);
        pfds[i].fd = block[i].timerfd;
        pfds[i].events = POLLIN;
        if (!block[i].decay) { // only non-decaying blocks have timers armed initially
            if (ARM_TIMER(block[i]) < 0)
                warn("ARM_TIMER: %d", errno);
        }
    }
}

void sigtoa (int n, char* ret) {
    ret[3] = 0;
    ret[2] = 0x0a;
    ret[1] = (n % 10) + 48;
    ret[0] = (n / 10) + 48;
}

void handler (int signo)
{
    int i = signo - SIGRTMIN;
    if (vflag) {
        char signr[21] = "Signal received: "; sigtoa(signo, signr+17);
        write(STDERR_FILENO, signr, 21);
    }
    if (i < 0) { /* standard signal received */
        finished = 1;
    }

    uint64_t msg = 0;
    write(block[i].timerfd, &msg, 8);
    if (block[i].decay) {
        ARM_TIMER(block[i]);
    }
}

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
    Window root = RootWindow(dpy, screen);

    int ready = 0;
    uint64_t waste;

    init();
    
    sigact.sa_handler = handler;
    for (int i = 0; i < 64; i++) {
        if (sigaction(i, &sigact, NULL) < 0 && vflag) /* certain signals will be ignored by default (0, 9, SIGSTOP, 32, 33) */
            warn("sigaction: (signal = %d) %d", i, errno);
    }

    /* main loop */
    while (!finished) {
        ready = poll(pfds, length, -1);
        if (ready) for (int i = 0; i<length; i++) {
            if (pfds[i].revents & POLLIN) {
                read(pfds[i].fd, &waste, 8); /* could be useful later */
                block[i].function(block[i].arg);
                if (strcmp(status[i], buffer))
                    strcpy(status[i], buffer);
            }
        }
        for (int j = length; j >= 0; j--) {
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
