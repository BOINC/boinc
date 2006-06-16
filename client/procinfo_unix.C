#include <stdio.h>
// see:
// man 5 proc
// /usr/src/linux/fs/proc/array.C

struct PROC_INFO {
    int pid;
    char comm[256];
    char state;
    int ppid;
    int pgrp;
    int session;
    int tty_nr;
    int tpgid;
    unsigned long flags;
    unsigned long minflt;
    unsigned long cminflt;
    unsigned long majflt;
    unsigned long cmajflt;
    unsigned long utime;
    unsigned long stime;
    int cutime;
    int cstime;
    int priority;
    int nice;
    int zero;
    int itrealvalue;
    unsigned long starttime;
    unsigned long vsize;
    int rss;
    unsigned long rlim;
    unsigned long startcode;
    unsigned long endcode;
    unsigned long startstack;
    unsigned long kstkesp;
    unsigned long kstkeip;
    unsigned long signal;
    unsigned long blocked;
    unsigned long sigignore;
    unsigned long sigcatch;
    unsigned long wchan;
    unsigned long nswap;
    unsigned long cnswap;
    int exit_signal;
    int processor;

    int parse(char*);
};

int PROC_INFO::parse(char* buf) {
    int n = sscanf(buf, "%d %s %c %d %d %d %d %d "
"%lu %lu %lu %lu %lu %lu %lu "
"%ld %ld %ld %ld %ld %ld "
"%lu %lu "
"%ld "
"%lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu "
"%d %d",
        &pid,
        &comm,
        &state,
        &ppid,
        &pgrp,
        &session,
        &tty_nr,
        &tpgid,
        &flags,
        &minflt,
        &cminflt,
        &majflt,
        &cmajflt,
        &utime,
        &stime,
        &cutime,
        &cstime,
        &priority,
        &nice,
        &zero,
        &itrealvalue,
        &starttime,
        &vsize,
        &rss,
        &rlim,
        &startcode,
        &endcode,
        &startstack,
        &kstkesp,
        &kstkeip,
        &signal,
        &blocked,
        &sigignore,
        &sigcatch,
        &wchan,
        &nswap,
        &cnswap
    );
    printf("read %d items\n", n);
    return 0;
}

int main() {
    PROC_INFO pi;
    char buf[1024];
    FILE* f = fopen("/proc/13690/stat", "r");
    fgets(buf, 1024, f);
    pi.parse(buf);
    printf("pid %d comm (%s) ut %lu st %lu cut %lu cst %lu\n",
            pi.pid, pi.comm,
        pi.utime, pi.stime, pi.cutime, pi.cstime
    );
}

