#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <getopt.h>
#include "init.h"
#include "runlevel.h"
#include "wall.h"

#define  FIFO_TIMEOUT 5

static int  halt(int runlevel, const char *name);
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
    while ((o = getopt(argc, argv, "ihdfnpw")) >= 0) {
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
        runlevel = parse_runlevel(*argv[optind]);
        /* ensure we get exactly one runlevel back */
        if (!runlevel || argv[optind][1]) {
            fprintf(stderr, "%s: not a runlevel: %s\n", name, argv[optind]);
            return 1;
        }
    }

    if (doit) {
        if (!runlevel) {
            fprintf(stderr, "%s: no runlevel specified\n", name);
            return 1;
        }
        warn(runlevel, 0);
        return halt(runlevel, name);
    }
    return 0;
}

static int halt(int runlevel, const char *name)
{
    /* open initctl FIFO, make sure we don't hang forever */
    sigset_t sigs;
    sigemptyset(&sigs);

    struct sigaction act;
    act.sa_handler = timeout;
    act.sa_mask    = sigs;
    act.sa_flags   = 0;
    sigaction(SIGALRM, &act, NULL);

    alarm(FIFO_TIMEOUT);
    FILE *f = fopen(SYSV_FIFO, "w");
    alarm(0);

    if (!f) {
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
    msg.runlevel  = emit_runlevel(runlevel);
    msg.sleeptime = SYSV_DEFAULT_SLEEP;
    if (fwrite(&msg, sizeof(msg), 1, f) < 1) {
        fprintf(stderr, "%s: can't write to %s\n", name, SYSV_FIFO);
        goto err;
    }
    fclose(f);

    return 0;
err:
    if (f)
        fclose(f);
    return 1;
}

static void timeout(int signal)
{
    timedout = 1;
}
