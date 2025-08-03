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

#define VERBOSE false

int main(int argc, char *argv[])
{
    char *args[128];
    int argsCount = 0;
    int argvCount;
    char buf [2048];
    char Podman_Path[1024];
    FILE *f;
    int status = 0;

    setuid(0);  // May not actually be needed
    if (getuid() != 0) {
        return -1;
    }

#if VERBOSE
    FILE *debug_file = fopen("/Users/Shared/Run_Podman-debug.txt", "a");
    fprintf(debug_file, "uid=%d, euid=%d\n", getuid(), geteuid());
#endif

    // While the official Podman installer puts the porman executable at
    // "/opt/podman/bin/podman", other installation methods (e.g. brew) might not
#if 1
    // The Mac installer wrote path to podman executable in Path_to_podman.txt
    Podman_Path[0] = 0;
    f = fopen("/Library/Application Support/BOINC Data/Path_to_podman.txt", "r");
    if (f) {
        fgets(Podman_Path, sizeof(Podman_Path), f);
        fclose(f);
    }
    if (Podman_Path[0] == 0) {
        // If we couldn't get it rom that file, use the default
        strlcpy(Podman_Path, "/opt/podman/bin/podman", sizeof(Podman_Path));
    }
#else
    // Get the path to the podman executable dynamically
    char                    s[2048];
    // Mac executables get a very limited PATH environment variable, so we must get the
    // PATH variable used by Terminal and search ther of the path to podman
    f = popen("a=`/usr/libexec/path_helper`;b=${a%\\\"*}\\\";env ${b} which podman", "r");
    s[0] = '\0';
    if (f) {
        fgets(s, sizeof(s), f);
        pclose(f);
        char * p=strstr(s, "\n");
        if (p) *p='\0'; // Remove the newline character
        fprintf(debug_file, "popen #2 returned podman path = \"%s\"\n", s);
        fclose(f);
    }
#endif

    // Podman always writes its files owned by the logged in user and
    // mosly in that user's home directory. To get around this, we
    // simulate a login by user boinc_project and set environment
    // variables for Podman to use our BOINC podman" directory instead
    args[argsCount++] = "su";
    args[argsCount++] = "-l";
    args[argsCount++] = "boinc_project";    // Create Podman VM using boinc_project so projects can access it
    args[argsCount++] = "-c";
    snprintf(buf, sizeof(buf), "env XDG_CONFIG_HOME=\"/Library/Application Support/BOINC podman\" XDG_DATA_HOME=\"/Library/Application Support/BOINC podman\" HOME=\"/Library/Application Support/BOINC podman\" %s", Podman_Path);

    argvCount = 1;   // arguments to be passed to Podman
    while (argv[argvCount]) {
        strlcat(buf, " ", sizeof(buf));
        strlcat(buf, argv[argvCount++], sizeof(buf));
    }
    args[argsCount++] = buf;
    args[argsCount++] = 0;

#if VERBOSE
    for (int i=0; i<argsCount; ++i) fprintf(debug_file,"args[%d] = %s\n", i, args[i]);
        printf("\n");
    fflush(debug_file);
#endif

    int pid = fork();
    if (pid == 0) {
        execvp(args[0], args);
        perror("execvp");
        fprintf(stderr, "couldn't exec %s: %d\n", args[0], errno);
        exit(errno);
    }
        waitpid(pid, &status, WUNTRACED);

#if VERBOSE
        fprintf(debug_file, "\n\n=========================\n\n");
        fclose(debug_file);
        chmod("/Users/Shared/Run_Podman-debug.txt", 0666);
#endif
    fflush(stdout);
    fflush(stderr);

    return 0;
}
