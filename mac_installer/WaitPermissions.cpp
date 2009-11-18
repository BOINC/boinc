// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2009 University of California
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
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

/* WaitPermissions.cpp */

#define CREATE_LOG 1    /* for debugging */

#include <Carbon/Carbon.h>

#include <sys/stat.h>   // for stat
#include <unistd.h>	// getuid

void print_to_log_file(const char *format, ...);
void strip_cr(char *buf);

// When we first create the boinc_master group and add the current user to the 
// new group, there is a delay before the new group membership is recognized.  
// If we launch the BOINC Manager too soon, it will fail with a -1037 permissions 
// error, so we wait until the current user can access the switcher application.
// Apparently, in order to get the changed permissions / group membership, we must 
// launch a new process belonging to the user.  It may also need to be in a new 
// process group or new session. Neither system() nor popen() works, even after 
// setting the uid and euid back to the logged in user, but LSOpenFSRef() does.
// This tiny application loops until it can access the switcher application.

int main(int argc, char *argv[])
{
    struct stat             sbuf;
    int                     i;
    int                     retval;
    
    for (i=0; i<180; i++) {     // Limit delay to 3 minutes
        retval = stat("/Library/Application Support/BOINC Data/switcher/switcher", &sbuf);
//        print_to_log_file("WaitPermissions: stat(switcher path) returned %d, uid = %d, euid = %d\n", retval, (int)getuid(), (int)geteuid());
        if (retval == 0) {
            return 0;
        }
        sleep(1);
    }
    return retval;
}

void strip_cr(char *buf)
{
    char *theCR;
    
    theCR = strrchr(buf, '\n');
    if (theCR)
        *theCR = '\0';
    theCR = strrchr(buf, '\r');
    if (theCR)
        *theCR = '\0';
}

// For debugging
void print_to_log_file(const char *format, ...) {
#if CREATE_LOG
    FILE *f;
    va_list args;
    char path[256], buf[256];
    time_t t;
    strcpy(path, "/Users/Shared/test_log.txt");
    //    strcpy(path, "/Users/");
    //    strcat(path, getlogin());
    //    strcat(path, "/Documents/test_log.txt");
    f = fopen(path, "a");
    if (!f) return;
    
    //  freopen(buf, "a", stdout);
    //  freopen(buf, "a", stderr);
    
    time(&t);
    strcpy(buf, asctime(localtime(&t)));
    strip_cr(buf);
    
    fputs(buf, f);
    fputs("   ", f);
    
    va_start(args, format);
    vfprintf(f, format, args);
    va_end(args);
    
    fputs("\n", f);
    fflush(f);
    fclose(f);
    chmod(path, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
    
#endif
}
