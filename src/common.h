#pragma once
#define DEFAULT_PATH  "/usr/sbin:/usr/bin:/sbin:/bin"
#define DEFAULT_TERM  "linux"
#define DEFAULT_UMASK 022
#define DEFAULT_USER  "root"

void prepenv();
void setprocname(const char *name, char *argv0);
