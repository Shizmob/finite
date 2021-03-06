#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "config.h"

void init(char *argv[])
{
    char *cmd = RC_COMMAND;
    for (; *argv; argv++)
        if (!strcmp(*argv, "rc="))
            cmd = strchr(*argv, '=') + 1;

    execv(cmd, (char *[]){ cmd, NULL, });
    fprintf(stderr, "init: exec %s: ", cmd);
    perror(NULL);
}
