#define  _XOPEN_SOURCE 700
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <getopt.h>
#include "init.h"
#include "inittab.h"

#define  FIFO_TIMEOUT 5

static void timeout(int sig);
static int  timedout;


int main(int argc, char *argv[])
{
    int runlevel = 0;

    /* try to get runlevel from program name */
    const char *name = strrchr(argv[0], '/');
    if (name)
        name++;
    else
        name = argv[0];

    if (!strcmp(name, "poweroff"))
        runlevel = RUNLEVEL_SHUTDOWN;
    else if (!strcmp(name, "reboot"))
        runlevel = RUNLEVEL_REBOOT;

    /* or from arguments */
    int doit = 1;
    char o;
    while ((o = getopt(argc, argv, "ihdfnpwt:")) >= 0) {
        switch (o) {
        case 'n':
        case 'f':
        case 'h':
        case 'i':
        case 'd':
            /* ignored, legacy*/
            break;
        case 'p':
            runlevel = RUNLEVEL_SHUTDOWN;
            break;
        case 'w':
            doit = 0;
            break;
        default:
            fprintf(stderr, "%s: unknown option: -%c\n", name, optopt);
            return 1;
        }
    }
    if (argv[optind]) {
        runlevel = parse_runlevels(argv[optind]);
        /* ensure we only get one runlevel back */
        if (!runlevel || (runlevel & (runlevel - 1))) {
            fprintf(stderr, "%s: not a runlevel: %s\n", name, argv[optind]);
            return 1;
        }
    }

    if (!runlevel) {
        fprintf(stderr, "%s: no runlevel specified\n", name);
        return 1;
    }

    /* actually change runlevel */
    if (doit) {
        /* open initctl FIFO, make sure we don't hang forever */
        sigset_t sigs;
        sigemptyset(&sigs);

        struct sigaction act;
        act.sa_handler = timeout;
        act.sa_mask    = sigs;
        act.sa_flags   = 0;
        sigaction(SIGALRM, &act, NULL);

        alarm(FIFO_TIMEOUT);
        int fd = open(SYSV_FIFO, O_WRONLY);
        alarm(0);
        if (fd < 0) {
            if (timedout)
                fprintf(stderr, "%s: can't open %s: timed out (%d seconds)\n", name, SYSV_FIFO, FIFO_TIMEOUT);
            else
                fprintf(stderr, "%s: can't open %s: %s\n", name, SYSV_FIFO, strerror(errno));
            goto err;
        }

        /* send runlevel message */
        struct sysv_message msg = { 0 };
        msg.magic     = SYSV_MESSAGE_MAGIC;
        msg.cmd       = SYSV_MESSAGE_RUNLEVEL;
        msg.runlevel  = runlevel;
        msg.sleeptime = SYSV_DEFAULT_SLEEP;
        if (write(fd, &msg, sizeof(msg)) != sizeof(msg)) {
            fprintf(stderr, "%s: can't write to %s\n", name, SYSV_FIFO);
            goto err;
        }

        return 0;
err:
        if (fd >= 0)
            close(fd);
        return 1;
    }
    return 0;
}

static void timeout(int signal)
{
    timedout = 1;
}
