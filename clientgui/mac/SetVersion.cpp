// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2014 University of California
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

/*
 *  SetVersion.cpp
 *  boinc
 *
 *  Created by Charlie Fenton on 3/29/05.
 *  Last updated by Charlie Fenton on 5/8/13.
 *
 */

// Set STAND_ALONE TRUE if testing as a separate applicaiton
#define STAND_ALONE 0
#define VERBOSE_SPAWN 0  /* for debugging callPosixSpawn */

#include <Carbon/Carbon.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/param.h>  // for MAXPATHLEN
#include <sys/stat.h>
#include "version.h"

int IsFileCurrent(char* filePath);
int file_exists(const char* path);
int FixInfoPlistFile(char* name);
int FixInfoPlist_Strings(char* myPath, char* name);
int MakeBOINCPackageInfoPlistFile(char* myPath, char* brand);
int MakeBOINCRestartPackageInfoPlistFile(char* myPath, char* brand);
int MakeMetaPackageInfoPlistFile(char* myPath, char* brand);
int callPosixSpawn(const char *cmd);

int main(int argc, char** argv) {
    int retval = 0, err;

#if STAND_ALONE
    char myPath[1024];
    getcwd(myPath, sizeof(myPath));
    printf("%s\n", myPath);       // For debugging
#endif

    if (!file_exists("./English.lproj")) {
        retval = mkdir("./English.lproj", 0755);
        if (retval) {
            printf("Error %d creating directory English.lproj\n", retval);
        }
    }
    
    // BOINC Manager
    err = FixInfoPlist_Strings("./English.lproj/InfoPlist.strings", "BOINC Manager");
    if (err) retval = err;
    err = FixInfoPlistFile("Info.plist");
    if (err) retval = err;
    
    // BOINC Installer
    err = FixInfoPlist_Strings("./English.lproj/Installer-InfoPlist.strings", "BOINC Installer");
    if (err) retval = err;
    err = FixInfoPlistFile("Installer-Info.plist");
    if (err) retval = err;
    
    // BOINC PostInstall app
    err = FixInfoPlist_Strings("./English.lproj/PostInstall-InfoPlist.strings", "Install BOINC");
    if (err) retval = err;
    err = FixInfoPlistFile("PostInstall-Info.plist");
    if (err) retval = err;
    
    // BOINC Screen Saver
    err = FixInfoPlist_Strings("./English.lproj/ScreenSaver-InfoPlist.strings", "BOINC Screen Saver");
    if (err) retval = err;
    err = FixInfoPlistFile("ScreenSaver-Info.plist");
    if (err) retval = err;
    
    // BOINC Uninstaller
    err = FixInfoPlist_Strings("./English.lproj/Uninstaller-InfoPlist.strings", "Uninstall BOINC");
    if (err) retval = err;
    err = FixInfoPlistFile("Uninstaller-Info.plist");
    if (err) retval = err;
    
    err = FixInfoPlistFile("SystemMenu-Info.plist");
    if (err) retval = err;

    // WaitPermissions is not currently used
    err = FixInfoPlistFile("WaitPermissions-Info.plist");
    if (err) retval = err;
    
    err = MakeBOINCPackageInfoPlistFile("./Pkg-Info.plist", "BOINC Manager");
    if (err) retval = err;
    
    err = MakeBOINCRestartPackageInfoPlistFile("./Pkg_Restart-Info.plist", "BOINC Manager");
    if (err) retval = err;
        
    err = MakeMetaPackageInfoPlistFile("./Mpkg-Info.plist", "BOINC Manager");
    return retval;
}


int IsFileCurrent(char* filePath) {
    FILE *f;
    char *c, buf[1024];
    
    f = fopen(filePath, "r");
    if (f == 0)
        return false;
    for (;;) {
        c = fgets(buf, sizeof(buf), f);
        if (c == NULL)
            break;   // EOF reached without finding correct version string
        c = strstr(buf, BOINC_VERSION_STRING);
        if (c) {
            fclose(f);
            return true;  // File contains current version string
        }
    }
    fclose(f);
    return false;  // File does not contain current version string
}


int file_exists(const char* path) {
    struct stat buf;
    if (stat(path, &buf)) {
        return false;     // stat() returns zero on success
    }
    return true;
}


int FixInfoPlist_Strings(char* myPath, char* name) {
    int retval = 0;
    FILE *f;
    time_t cur_time;
    struct tm *time_data;

    cur_time = time(NULL);
    time_data = localtime( &cur_time );
    
    if (IsFileCurrent(myPath))
        return 0;

    f = fopen(myPath, "w");
    if (f)
    {
        fprintf(f, "/* Localized versions of Info.plist keys */\n\n");
        fprintf(f, "CFBundleName = \"%s\";\n", name);
        fprintf(f, "CFBundleShortVersionString = \"%s version %s\";\n", name, BOINC_VERSION_STRING);
        fprintf(f, "CFBundleGetInfoString = \"%s version %s, Copyright %d University of California.\";\n", name, BOINC_VERSION_STRING, time_data->tm_year+1900);
        fflush(f);
        retval = fclose(f);
    }
    else {
        printf("Error creating file %s\n", myPath);
        retval = -1;
    }
        
    return retval;
}

int FixInfoPlistFile(char* name) {
    int retval = 0;
    FILE *fin = NULL, *fout = NULL;
    char *c, a, buf[1024];
    char srcPath[MAXPATHLEN], dstPath[MAXPATHLEN];

    strcpy(dstPath, "./");
    strcat(dstPath, name);
    
    strcpy(srcPath, "../clientgui/mac/templates/");
    strcat(srcPath, name);
    
    if (IsFileCurrent(dstPath))
        return 0;

    // Save the old file in case there is an error updating it
    if (file_exists(dstPath)) {
        rename(dstPath, "./temp");
    }

    fin = fopen(srcPath, "r");
    if (fin == NULL)
        goto bail;

    fout = fopen(dstPath, "w");
    if (fout == NULL) {
        goto bail;
    }

    // Copy everything up to version number
    for (;;) {
        c = fgets(buf, sizeof(buf), fin);
        if (c == NULL)
            goto bail;   // EOF
        c = strstr(buf, "CFBundleVersion</key>");
        if (c)
            break;  // Found "CFBundleVersion</key>"
        fputs(buf, fout);
    }
        
    c = strstr(buf, "<string>");
    if (c == NULL) {
        fputs(buf, fout);
        c = fgets(buf, sizeof(buf), fin);
        if (c == NULL)
            goto bail;   // EOF
        c = strstr(buf, "<string>");
        if (c == NULL)
            goto bail;   // "CFBundleVersion</key>" not followed by "<string>"
    }
    
    a = *(c+8);
    *(c+8) = '\0';                      // Put terminator after "<string>"
    fputs(buf, fout);                   // Copy up to end of "<string>"
    fputs(BOINC_VERSION_STRING, fout);  // Write the current version number
    *(c+8) = a;                         // Undo terminator we inserted
    c = strstr(buf, "</string>");       // Skip over old version number in input
    fputs(c, fout);                     // Copy rest of input line

    // Copy rest of file
    for (;;) {
        c = fgets(buf, sizeof(buf), fin);
        if (c == NULL)
            break;   // EOF
        fputs(buf, fout);
    }

    fclose(fin);
    fflush(fout);
    fclose(fout);
        
    unlink("temp");
    
    return retval;

bail:
    if (fin)
        fclose(fin);
    if (fout)
        fclose(fout);

    if (file_exists("./temp")) {
        rename("./temp", dstPath);
//    sprintf(buf, "mv -f temp %s", myPath);
//    retval = callPosixSpawn(buf);
    } else {
        sprintf(buf, "cp -f %s %s", srcPath, dstPath);
        retval = callPosixSpawn(buf);
    }
    
    printf("Error updating version number in file %s\n", dstPath);
    return -1;
}


int MakeBOINCPackageInfoPlistFile(char* myPath, char* brand) {
    int retval = 0;
    FILE *f;
    
    if (IsFileCurrent(myPath))
        return 0;

    f = fopen(myPath, "w");
    if (f)
    {
        fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
        fprintf(f, "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n");
        fprintf(f, "<plist version=\"1.0\">\n<dict>\n");
        fprintf(f, "\t<key>CFBundleGetInfoString</key>\n");
        fprintf(f, "\t<string>%s %s</string>\n", brand, BOINC_VERSION_STRING);
        fprintf(f, "\t<key>CFBundleIdentifier</key>\n\t<string>edu.berkeley.boinc</string>\n");
        fprintf(f, "\t<key>CFBundleShortVersionString</key>\n");
        fprintf(f, "\t<string>%s</string>\n", BOINC_VERSION_STRING);
        fprintf(f, "\t<key>IFPkgFlagAllowBackRev</key>\n\t<integer>1</integer>\n");
        fprintf(f, "\t<key>IFPkgFlagAuthorizationAction</key>\n\t<string>AdminAuthorization</string>\n");
        fprintf(f, "\t<key>IFPkgFlagDefaultLocation</key>\n\t<string>/</string>\n");
        fprintf(f, "\t<key>IFPkgFlagFollowLinks</key>\n\t<integer>0</integer>\n");
        fprintf(f, "\t<key>IFPkgFlagInstallFat</key>\n\t<integer>0</integer>\n");
        fprintf(f, "\t<key>IFPkgFlagInstalledSize</key>\n\t<integer>6680</integer>\n");
        fprintf(f, "\t<key>IFPkgFlagIsRequired</key>\n\t<integer>0</integer>\n");
        fprintf(f, "\t<key>IFPkgFlagOverwritePermissions</key>\n\t<integer>0</integer>\n");
        fprintf(f, "\t<key>IFPkgFlagRelocatable</key>\n\t<integer>0</integer>\n");
        fprintf(f, "\t<key>IFPkgFlagRestartAction</key>\n\t<string>NoRestart</string>\n");
        fprintf(f, "\t<key>IFPkgFlagRootVolumeOnly</key>\n\t<integer>1</integer>\n");
        fprintf(f, "\t<key>IFPkgFlagUpdateInstalledLanguages</key>\n\t<integer>0</integer>\n");
        fprintf(f, "\t<key>IFPkgFormatVersion</key>\n\t<real>0.10000000149011612</real>\n");
        fprintf(f, "</dict>\n</plist>\n");

        fflush(f);
        retval = fclose(f);
    }
    else {
        printf("Error creating file %s\n", myPath);
        retval = -1;
    }
        
    return retval;
}


// Create a MetaPackage whcih runs only BOINC,pkg but specifies Restart Required
int MakeBOINCRestartPackageInfoPlistFile(char* myPath, char* brand) {
    int retval = 0;
    FILE *f;
    
    if (IsFileCurrent(myPath))
        return 0;

    f = fopen(myPath, "w");
    if (f)
    {
        fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
        fprintf(f, "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n");
        fprintf(f, "<plist version=\"1.0\">\n<dict>\n");
        fprintf(f, "\t<key>CFBundleGetInfoString</key>\n");
        fprintf(f, "\t<string>%s %s</string>\n", brand, BOINC_VERSION_STRING);
        fprintf(f, "\t<key>CFBundleIdentifier</key>\n\t<string>edu.berkeley.boinc_r</string>\n");
        fprintf(f, "\t<key>CFBundleShortVersionString</key>\n");
        fprintf(f, "\t<string>%s</string>\n", BOINC_VERSION_STRING);
        fprintf(f, "\t<key>IFMajorVersion</key>\n\t<integer>%d</integer>\n", BOINC_MAJOR_VERSION);
        fprintf(f, "\t<key>IFMinorVersion</key>\n\t<integer>%d</integer>\n", BOINC_MINOR_VERSION);
        fprintf(f, "\t<key>IFPkgFlagAllowBackRev</key>\n\t<integer>1</integer>\n");
        fprintf(f, "\t<key>IFPkgFlagAuthorizationAction</key>\n\t<string>AdminAuthorization</string>\n");
        fprintf(f, "\t<key>IFPkgFlagRestartAction</key>\n\t<string>RequiredRestart</string>\n");
        fprintf(f, "\t<key>IFPkgFlagRootVolumeOnly</key>\n\t<integer>1</integer>\n");
        fprintf(f, "\t<key>IFPkgFlagComponentDirectory</key>\n\t<string>../</string>\n");

        fprintf(f, "\t<key>IFPkgFlagPackageList</key>\n");
        
        fprintf(f, "\t<array>\n");
        fprintf(f, "\t\t<dict>\n");
        fprintf(f, "\t\t\t<key>IFPkgFlagPackageLocation</key>\n\t\t\t<string>BOINC.pkg</string>\n");
        fprintf(f, "\t\t\t<key>IFPkgFlagPackageSelection</key>\n\t\t\t<string>required</string>\n");
        fprintf(f, "\t\t</dict>\n");
        fprintf(f, "\t</array>\n");

        fprintf(f, "\t<key>IFPkgFormatVersion</key>\n\t<real>0.10000000149011612</real>\n");
        fprintf(f, "</dict>\n</plist>\n");

        fflush(f);
        retval = fclose(f);
    }
    else {
        printf("Error creating file %s\n", myPath);
        retval = -1;
    }
        
    return retval;
}


// Make a MetaPackage to install both BOINC and VirtualBox
int MakeMetaPackageInfoPlistFile(char* myPath, char* brand) {
    int retval = 0;
    FILE *f;
    
    if (IsFileCurrent(myPath))
        return 0;

    f = fopen(myPath, "w");
    if (f)
    {
        fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
        fprintf(f, "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n");
        fprintf(f, "<plist version=\"1.0\">\n<dict>\n");
        fprintf(f, "\t<key>CFBundleGetInfoString</key>\n");
        fprintf(f, "\t<string>%s %s + VirtualBox</string>\n", brand, BOINC_VERSION_STRING);
        fprintf(f, "\t<key>CFBundleIdentifier</key>\n\t<string>edu.berkeley.boinc+vbox</string>\n");
        fprintf(f, "\t<key>CFBundleShortVersionString</key>\n");
        fprintf(f, "\t<string>%s</string>\n", BOINC_VERSION_STRING);
        fprintf(f, "\t<key>IFMajorVersion</key>\n\t<integer>%d</integer>\n", BOINC_MAJOR_VERSION);
        fprintf(f, "\t<key>IFMinorVersion</key>\n\t<integer>%d</integer>\n", BOINC_MINOR_VERSION);
        fprintf(f, "\t<key>IFPkgFlagAllowBackRev</key>\n\t<integer>1</integer>\n");
        fprintf(f, "\t<key>IFPkgFlagAuthorizationAction</key>\n\t<string>AdminAuthorization</string>\n");
        fprintf(f, "\t<key>IFPkgFlagRestartAction</key>\n\t<string>NoRestart</string>\n");
        fprintf(f, "\t<key>IFPkgFlagRootVolumeOnly</key>\n\t<integer>1</integer>\n");
        fprintf(f, "\t<key>IFPkgFlagComponentDirectory</key>\n\t<string>../</string>\n");

        fprintf(f, "\t<key>IFPkgFlagPackageList</key>\n");
        
        fprintf(f, "\t<array>\n");
        fprintf(f, "\t\t<dict>\n");
        fprintf(f, "\t\t\t<key>IFPkgFlagPackageLocation</key>\n\t\t\t<string>BOINC.pkg</string>\n");
        fprintf(f, "\t\t\t<key>IFPkgFlagPackageSelection</key>\n\t\t\t<string>required</string>\n");
        fprintf(f, "\t\t</dict>\n");

        fprintf(f, "\t\t<dict>\n");
        fprintf(f, "\t\t\t<key>IFPkgFlagPackageLocation</key>\n\t\t\t<string>VirtualBox.pkg</string>\n");
        fprintf(f, "\t\t\t<key>IFPkgFlagPackageSelection</key>\n\t\t\t<string>selected</string>\n");
        fprintf(f, "\t\t</dict>\n");
        fprintf(f, "\t</array>\n");

        fprintf(f, "\t<key>IFPkgFormatVersion</key>\n\t<real>0.10000000149011612</real>\n");
        fprintf(f, "</dict>\n</plist>\n");

        fflush(f);
        retval = fclose(f);
    }
    else {
        printf("Error creating file %s\n", myPath);
        retval = -1;
    }
        
    return retval;
}




#define NOT_IN_TOKEN                0
#define IN_SINGLE_QUOTED_TOKEN      1
#define IN_DOUBLE_QUOTED_TOKEN      2
#define IN_UNQUOTED_TOKEN           3

static int parse_posic_spawn_command_line(char* p, char** argv) {
    int state = NOT_IN_TOKEN;
    int argc=0;

    while (*p) {
        switch(state) {
        case NOT_IN_TOKEN:
            if (isspace(*p)) {
            } else if (*p == '\'') {
                p++;
                argv[argc++] = p;
                state = IN_SINGLE_QUOTED_TOKEN;
                break;
            } else if (*p == '\"') {
                p++;
                argv[argc++] = p;
                state = IN_DOUBLE_QUOTED_TOKEN;
                break;
            } else {
                argv[argc++] = p;
                state = IN_UNQUOTED_TOKEN;
            }
            break;
        case IN_SINGLE_QUOTED_TOKEN:
            if (*p == '\'') {
                if (*(p-1) == '\\') break;
                *p = 0;
                state = NOT_IN_TOKEN;
            }
            break;
        case IN_DOUBLE_QUOTED_TOKEN:
            if (*p == '\"') {
                if (*(p-1) == '\\') break;
                *p = 0;
                state = NOT_IN_TOKEN;
            }
            break;
        case IN_UNQUOTED_TOKEN:
            if (isspace(*p)) {
                *p = 0;
                state = NOT_IN_TOKEN;
            }
            break;
        }
        p++;
    }
    argv[argc] = 0;
    return argc;
}


#include <spawn.h>

int callPosixSpawn(const char *cmdline) {
    char command[1024];
    char progName[1024];
    char progPath[MAXPATHLEN];
    char* argv[100];
    int argc = 0;
    char *p;
    pid_t thePid = 0;
    int result = 0;
    int status = 0;
    extern char **environ;
    
    // Make a copy of cmdline because parse_posic_spawn_command_line modifies it
    strlcpy(command, cmdline, sizeof(command));
    argc = parse_posic_spawn_command_line(const_cast<char*>(command), argv);
    strlcpy(progPath, argv[0], sizeof(progPath));
    strlcpy(progName, argv[0], sizeof(progName));
    p = strrchr(progName, '/');
    if (p) {
        argv[0] = p+1;
    } else {
        argv[0] = progName;
    }
    
#if VERBOSE_SPAWN
    printf("***********");
    for (int i=0; i<argc; ++i) {
        printf("argv[%d]=%s", i, argv[i]);
    }
    printf("***********\n");
#endif

    errno = 0;

    result = posix_spawnp(&thePid, progPath, NULL, NULL, argv, environ);
#if VERBOSE_SPAWN
    printf("callPosixSpawn command: %s", cmdline);
    printf("callPosixSpawn: posix_spawnp returned %d: %s", result, strerror(result));
#endif
    if (result) {
        return result;
    }
// CAF    int val =
    waitpid(thePid, &status, WUNTRACED);
// CAF        if (val < 0) printf("first waitpid returned %d\n", val);
    if (status != 0) {
#if VERBOSE_SPAWN
        printf("waitpid() returned status=%d", status);
#endif
        result = status;
    } else {
        if (WIFEXITED(status)) {
            result = WEXITSTATUS(status);
            if (result == 1) {
#if VERBOSE_SPAWN
                printf("WEXITSTATUS(status) returned 1, errno=%d: %s", errno, strerror(errno));
#endif
                result = errno;
            }
#if VERBOSE_SPAWN
            else if (result) {
                printf("WEXITSTATUS(status) returned %d", result);
            }
#endif
        }   // end if (WIFEXITED(status)) else
    }       // end if waitpid returned 0 sstaus else
    
    return result;
}
