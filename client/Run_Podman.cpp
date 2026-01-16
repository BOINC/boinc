// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2025 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <https://www.gnu.org/licenses/>.

// Run_Podman.cpp
//
// This must be called setuid root.
// The BOINC installer for Macintosh sets this executable's permissions
// to include the setuid bit and its owner / user to root.
//

#include <unistd.h>
#include <stdio.h>
#include <cstring>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <pwd.h>

#define VERBOSE false


int main(int argc, char *argv[])
{
    passwd *pw;
    bool runWithoutSandbox = false;
    char *args[128];
    int argsCount = 0;
    int argvCount;
    char buf [2048];
    int status = 0;

    if (geteuid() != 0) {
        pw = getpwnam("boinc_project");
        if (!pw) {
            runWithoutSandbox =  true;
        } else {
            fprintf(stderr, "ERROR: Run_Podman must be called setuid root\n");
            exit(1);
        }
    } else {
        setuid(0);  // May not actually be needed
    }

#if VERBOSE
    FILE *debug_file = fopen("/Users/Shared/Run_Podman-debug.txt", "a");
    fprintf(debug_file, "uid=%d, euid=%d, gid=%d, egid=%d\n", getuid(), geteuid(), getgid(), getegid());
    chmod("/Users/Shared/Run_Podman-debug.txt", 0666);

    int saved_stderr_fd = dup(fileno(stderr));
    freopen("/Users/Shared/stderr_Run_Podman.txt", "a", stderr);
    chmod("/Users/Shared/stderr_Run_Podman.txt", 0666);
    fprintf(stderr, "\n========\n");
    fflush(stderr);

    int saved_stdout_fd = dup(fileno(stdout));
    freopen("/Users/Shared/stdout_Run_Podman.txt", "a", stdout);
    chmod("/Users/Shared/stdout_Run_Podman.txt", 0666);
    fprintf(stdout, "\n********\n");
    fflush(stdout);
#endif


#if VERBOSE
    dup2(saved_stdout_fd, fileno(stdout));
    close(saved_stdout_fd); // Close the saved descriptor
    dup2(saved_stderr_fd, fileno(stderr));
    close(saved_stderr_fd); // Close the saved descriptor
#endif

    // Podman always writes its files owned by the logged in user and
    // mostly in that user's home directory. To get around this, we
    // simulate a login by user boinc_project and set environment
    // variables for Podman to use our BOINC podman" directory instead
    strlcpy(buf, "env XDG_CONFIG_HOME=\"/Library/Application Support/BOINC podman\" XDG_DATA_HOME=\"/Library/Application Support/BOINC podman\" HOME=\"/Library/Application Support/BOINC podman\" ", sizeof(buf));

    argvCount = 1;   // arguments to be passed to Podman
    while (argv[argvCount]) {
        strlcat(buf, " ", sizeof(buf));
        strlcat(buf, argv[argvCount++], sizeof(buf));
    }

#if VERBOSE
    for (int i=0; i<argsCount; ++i) fprintf(debug_file,"args[%d] = %s\n", i, args[i]);
    fprintf(debug_file,"command: %s\n", buf);
    fflush(debug_file);
#endif

    if (runWithoutSandbox) {
        system(buf);
    } else {
        args[argsCount++] = "su";
        args[argsCount++] = "-l";
        args[argsCount++] = "boinc_project";    // Create Podman VM using boinc_project so projects can access it
        args[argsCount++] = "-c";
        args[argsCount++] = buf;
        args[argsCount++] = 0;

#if VERBOSE
        for (int i=0; i<argsCount; ++i) fprintf(debug_file,"args[%d] = %s\n", i, args[i]);
        fprintf(debug_file,"\n");
#endif

        int pid = fork();
        if (pid == 0) {
            execvp(args[0], args);
            perror("execvp");
            fprintf(stderr, "couldn't exec %s: %d\n", args[0], errno);
#if VERBOSE
            fprintf(debug_file, "couldn't exec %s: %d\n", args[0], errno);
            fflush(debug_file);
            fclose(debug_file);
#endif
            exit(errno);
        }
        waitpid(pid, &status, WUNTRACED);
    }   // (runWithoutSandbox == false)
#if VERBOSE
    int myerr = WEXITSTATUS(status);
    fprintf(stderr, "Run_Podman waitpid returned status %d: %s for cmd %s\n", myerr, strerror(myerr), buf);
    fprintf(debug_file, "\n\n=========================\n\n");
    fflush(debug_file);
    fclose(debug_file);
    chmod("/Users/Shared/Run_Podman-debug.txt", 0666);
#endif

    fflush(stdout);
    fflush(stderr);

    return 0;
}
