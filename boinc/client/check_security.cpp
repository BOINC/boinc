// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
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

// check_security.C


#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>    // getpwnam
#include <grp.h>
#include <dirent.h>
#include <cerrno>

#if (defined(__APPLE__) && defined(_DEBUG))
#include <Carbon/Carbon.h>
#endif

#include "util.h"
#include "error_numbers.h"
#include "file_names.h"
#include "filesys.h"

#ifdef __WXMAC__                            // If Mac BOINC Manager
bool IsUserInGroupBM();
#endif

static int CheckNestedDirectories(char * basepath, int depth, 
                                    int use_sandbox, int isManager, 
                                    char * path_to_error
                                );

#if (! defined(__WXMAC__) && ! defined(_MAC_INSTALLER))
static char * PersistentFGets(char *buf, size_t buflen, FILE *f);
static void GetPathToThisProcess(char* outbuf, size_t maxLen);
#endif

#define REAL_BOINC_MASTER_NAME "boinc_master"
#define REAL_BOINC_PROJECT_NAME "boinc_project"


static char         boinc_master_user_name[64];
static char         boinc_master_group_name[64];
static char         boinc_project_user_name[64];
static char         boinc_project_group_name[64];

static gid_t        boinc_master_gid, boinc_project_gid;
static uid_t        boinc_master_uid, boinc_project_uid;

// Called from BOINC Manager, BOINC Client and Installer.
// The arguments are use only when called from the Installer

// Returns FALSE (0) if owners and permissions are OK, else TRUE (1)
int check_security(
#ifdef _MAC_INSTALLER
char *bundlePath, char *dataPath,
#endif
int use_sandbox, int isManager, char* path_to_error
) {
    passwd              *pw;
    group               *grp;
    gid_t               egid;
    uid_t               euid;
    char                dir_path[MAXPATHLEN], full_path[MAXPATHLEN];
    struct stat         sbuf;
    int                 retval;
    int                 useFakeProjectUserAndGroup = 0;
#if (defined(__APPLE__) && defined(_DEBUG))
    long                response;
    OSStatus            err = noErr;
#endif
#ifdef __WXMAC__                            // If Mac BOINC Manager
    ProcessSerialNumber ourPSN;
    ProcessInfoRec      pInfo;
    FSRef               ourFSRef;
#endif

#define NUMBRANDS 3

char *saverName[NUMBRANDS];

saverName[0] = "BOINCSaver";
saverName[1] = "GridRepublic";
saverName[2] = "Progress Thru Processors";

    useFakeProjectUserAndGroup = ! use_sandbox;
#ifdef _DEBUG
#ifdef DEBUG_WITH_FAKE_PROJECT_USER_AND_GROUP
        useFakeProjectUserAndGroup = 1;
#endif
#ifdef __APPLE__
    err = Gestalt(gestaltSystemVersion, (SInt32*)&response);
    if ((err == noErr) && (response < 0x1040))
        useFakeProjectUserAndGroup = 1;
#endif      // __APPLE__
#endif      // _DEBUG

// GDB can't attach to applications which are running as a diferent user or group so
//  it ignores the S_ISUID and S_ISGID permisison bits when launching an application.
// To work around this, and to allow testing the uninstalled Deployment version, we
//  assume that the BOINC Client has the correct user and group. 
// We must get the BOINC Client's user and group differently depending on whether we
//  were called from the Manager or from the Client

#ifdef __WXMAC__                            // If Mac BOINC Manager
    // Get the full path to BOINC Manager application's bundle
    retval = GetCurrentProcess (&ourPSN);
    if (retval)
        return -1000;          // Should never happen

    memset(&pInfo, 0, sizeof(pInfo));
    pInfo.processInfoLength = sizeof( ProcessInfoRec );
    retval = GetProcessInformation(&ourPSN, &pInfo);
    if (retval)
        return -1001;          // Should never happen

    retval = GetProcessBundleLocation(&ourPSN, &ourFSRef);
    if (retval)
        return -1002;          // Should never happen

    retval = FSRefMakePath (&ourFSRef, (UInt8*)dir_path, sizeof(dir_path));
    if (retval)
        return -1003;          // Should never happen
#elif defined (_MAC_INSTALLER)
    strlcpy(dir_path, bundlePath, sizeof(dir_path));
#endif

    if (use_sandbox) {
#if (defined(__WXMAC__) || defined(_MAC_INSTALLER)) // If called from Mac BOINC Manager or installer
        // Get the full path to BOINC Client inside this application's bundle
        strlcpy(full_path, dir_path, sizeof(full_path));
        strlcat(full_path, "/Contents/Resources/boinc", sizeof(full_path));
#else
    if (isManager) {                                // If called from BOINC Manager but not on Mac
        getcwd(full_path, sizeof(full_path));       // Assume Client is in current directory
        strlcat(full_path, "/boinc", sizeof(full_path));
    } else                                          // If called from BOINC Client
        GetPathToThisProcess(full_path, sizeof(full_path));
#endif

        retval = stat(full_path, &sbuf);
        if (retval)
            return -1004;          // Should never happen

        if ((sbuf.st_mode & (S_ISUID | S_ISGID)) != (S_ISUID | S_ISGID))
            return -1005;

        boinc_master_uid = sbuf.st_uid;
        boinc_master_gid = sbuf.st_gid;

#ifdef __WXMAC__
    if (!IsUserInGroupBM())
        return -1099;
#endif
    } else {
        boinc_master_uid = geteuid();
        boinc_master_gid = getegid();
    }

#ifdef _MAC_INSTALLER
   // Require absolute owner and group boinc_master:boinc_master
    strlcpy(boinc_master_user_name, REAL_BOINC_MASTER_NAME, sizeof(boinc_master_user_name));
    pw = getpwnam(boinc_master_user_name);
    if (pw == NULL)
        return -1006;      // User boinc_master does not exist
    boinc_master_uid = pw->pw_uid;

    strlcpy(boinc_master_group_name, REAL_BOINC_MASTER_NAME, sizeof(boinc_master_group_name));
    grp = getgrnam(boinc_master_group_name);
    if (grp == NULL)
        return -1007;                // Group boinc_master does not exist
    boinc_master_gid = grp->gr_gid;

#else   // Use current user and group (see comment above)

    pw = getpwuid(boinc_master_uid);
    if (pw == NULL)
        return -1008;      // Should never happen
    strlcpy(boinc_master_user_name, pw->pw_name, sizeof(boinc_master_user_name));

    grp = getgrgid(boinc_master_gid);
    if (grp == NULL)
        return -1009;
    strlcpy(boinc_master_group_name, grp->gr_name, sizeof(boinc_master_group_name));

#endif

    if (useFakeProjectUserAndGroup) {
        // For easier debugging of project applications
        strlcpy(boinc_project_user_name, boinc_master_user_name, sizeof(boinc_project_user_name));
        strlcpy(boinc_project_group_name, boinc_master_group_name, sizeof(boinc_project_group_name));
        boinc_project_uid = boinc_master_uid;
        boinc_project_gid = boinc_master_gid;
    } else {
        strlcpy(boinc_project_user_name, REAL_BOINC_PROJECT_NAME, sizeof(boinc_project_user_name));
        pw = getpwnam(boinc_project_user_name);
        if (pw == NULL)
            return -1010;      // User boinc_project does not exist
        boinc_project_uid = pw->pw_uid;

        strlcpy(boinc_project_group_name, REAL_BOINC_PROJECT_NAME, sizeof(boinc_project_group_name));
        grp = getgrnam(boinc_project_group_name);
        if (grp == NULL)
            return -1011;                // Group boinc_project does not exist
        boinc_project_gid = grp->gr_gid;
    }

#if 0   // Manager is no longer setgid
#if (defined(__WXMAC__) || defined(_MAC_INSTALLER)) // If Mac BOINC Manager or installer
        // Get the full path to BOINC Manager executable inside this application's bundle
        strlcpy(full_path, dir_path, sizeof(full_path));
        strlcat(full_path, "/Contents/MacOS/", sizeof(full_path));
        // To allow for branding, assume name of executable inside bundle is same as name of bundle
        p = strrchr(dir_path, '/');         // Assume name of executable inside bundle is same as name of bundle
        if (p == NULL)
            p = dir_path - 1;
        strlcat(full_path, p+1, sizeof(full_path));
        p = strrchr(full_path, '.');         // Strip off  bundle extension (".app")
        if (p)
            *p = '\0'; 

        retval = lstat(full_path, &sbuf);
        if (retval)
            return -1013;          // Should never happen
        
        if (sbuf.st_gid != boinc_master_gid)
            return -1014;

        if (use_sandbox) {
            if ((sbuf.st_mode & S_ISGID) != S_ISGID)
                return -1015;
        }
#endif
#endif   // Manager is no longer setgid


#ifdef _MAC_INSTALLER
        // Require absolute owner and group boinc_master:boinc_master
        // Get the full path to BOINC Client inside this application's bundle
        strlcpy(full_path, dir_path, sizeof(full_path));
        strlcat(full_path, "/Contents/Resources/boinc", sizeof(full_path));

        retval = stat(full_path, &sbuf);
        if (retval)
            return -1016;          // Should never happen
        
        if (sbuf.st_gid != boinc_master_gid)
            return -1017;

        if (sbuf.st_uid != boinc_master_uid)
            return -1018;
#endif

#if (defined(__WXMAC__) || defined(_MAC_INSTALLER)) // If Mac BOINC Manager or installer
        // Version 6 screensaver has its own embedded switcher application, but older versions don't.
        // We don't allow unauthorized users to run the standard switcher application in the BOINC 
        // Data directory because they could use it to run as user & group boinc_project and damage 
        // project files.  The screensaver's gfx_switcher application has very limited functionality 
        // to avoid this risk.

        if (use_sandbox) {
            for (int i=0; i<NUMBRANDS; i++) {
                // Does gfx_switcher exist in screensaver bundle?
                sprintf(full_path, "/Library/Screen Savers/%s.saver/Contents/Resources/gfx_switcher", saverName[i]);
                retval = stat(full_path, &sbuf);
                if (! retval) {
#ifdef _DEBUG
                    if (sbuf.st_uid != boinc_master_uid)
                        return -1101;

                    if (sbuf.st_gid != boinc_master_gid)
                        return -1102;

                    if ((sbuf.st_mode & (S_ISUID | S_ISGID)) != 0)
                        return -1103;
#else
                    if (sbuf.st_uid != 0)
                        return -1101;

                    if (sbuf.st_gid != boinc_master_gid)
                        return -1102;

                    if ((sbuf.st_mode & (S_ISUID | S_ISGID)) != S_ISUID)
                        return -1103;
#endif
                }
            }
        }
#endif


//    rgid = getgid();
//    ruid = getuid();
    egid = getegid();
    euid = geteuid();

#ifdef _MAC_INSTALLER
    strlcpy(dir_path, dataPath, sizeof(dir_path));  // Installer
#else       // _MAC_INSTALLER
    getcwd(dir_path, sizeof(dir_path));             // Client or Manager

    if (! isManager) {                                   // If BOINC Client
    if (egid != boinc_master_gid)
            return -1019;     // Client should be running setgid boinc_master

        if (euid != boinc_master_uid)
            return -1020;     // BOINC Client should be running setuid boinc_master
    }
#endif

    retval = stat(dir_path, &sbuf);
    if (retval)
        return -1021;          // Should never happen

    if (use_sandbox) {

        // The top-level BOINC Data directory can have a different user if created by the Manager, 
        // but it should always have group boinc_master.
        if (sbuf.st_gid != boinc_master_gid)
            return -1022;
        
        // The top-level BOINC Data directory should have permission 771 or 571
        if ((sbuf.st_mode & 0577) != 0571)
            return -1023;
            
    } else {

        if (sbuf.st_uid != boinc_master_uid)
            return -1022;

    }

    strlcpy(full_path, dir_path, sizeof(full_path));
    strlcat(full_path, "/", sizeof(full_path));
    strlcat(full_path, PROJECTS_DIR, sizeof(full_path));
    retval = stat(full_path, &sbuf);
    if (! retval) {                 // Client can create projects directory if it does not yet exist.  
        if (use_sandbox) {
            if (sbuf.st_gid != boinc_project_gid)
                return -1024;

        if ((sbuf.st_mode & 0777) != 0770)
            return -1025;
        }
        
        if (sbuf.st_uid != boinc_master_uid)
            return -1026;

        // Step through project directories
        retval = CheckNestedDirectories(full_path, 1, use_sandbox, isManager, path_to_error);
        if (retval)
            return retval;
    }

    strlcpy(full_path, dir_path, sizeof(full_path));
    strlcat(full_path, "/", sizeof(full_path));
    strlcat(full_path, SLOTS_DIR, sizeof(full_path));
    retval = stat(full_path, &sbuf);
    if (! retval) {                 // Client can create slots directory if it does not yet exist.  
       if (use_sandbox) {
            if (sbuf.st_gid != boinc_project_gid)
                return -1027;

            if ((sbuf.st_mode & 0777) != 0770)
                return -1028;
        }
        
        if (sbuf.st_uid != boinc_master_uid)
            return -1029;

        // Step through slot directories
        retval = CheckNestedDirectories(full_path, 1, use_sandbox, isManager, path_to_error);
        if (retval)
            return retval;
    }

    strlcpy(full_path, dir_path, sizeof(full_path));
    strlcat(full_path, "/", sizeof(full_path));
    strlcat(full_path, GUI_RPC_PASSWD_FILE, sizeof(full_path));
    retval = stat(full_path, &sbuf);
    if (! retval) {                 // Client can create RPC password file if it does not yet exist.  
        if (use_sandbox) {
            if (sbuf.st_gid != boinc_master_gid)
                return -1030;

            if ((sbuf.st_mode & 0777) != 0660)
                return -1032;
        } else {
            if ((sbuf.st_mode & 0717) != 0600)
                return -1032;
        }
        
        if (sbuf.st_uid != boinc_master_uid)
            return -1031;
    }

    if (use_sandbox) {
        strlcpy(full_path, dir_path, sizeof(full_path));
        strlcat(full_path, "/", sizeof(full_path));
        strlcat(full_path, SWITCHER_DIR, sizeof(full_path));
        retval = stat(full_path, &sbuf);
        if (retval)
            return -1033;
          
        if (sbuf.st_gid != boinc_master_gid)
            return -1034;

        if (sbuf.st_uid != boinc_master_uid)
            return -1035;

        if ((sbuf.st_mode & 0777) != 0550)
            return -1036;

        strlcat(full_path, "/", sizeof(full_path));
        strlcat(full_path, SWITCHER_FILE_NAME, sizeof(full_path));
        retval = stat(full_path, &sbuf);
        if (retval)
            return -1037;
        
        if (sbuf.st_gid != boinc_master_gid)
            return -1038;

        if (sbuf.st_uid != 0)   // root
            return -1039;

        if ((sbuf.st_mode & 07777) != 04050)
            return -1040;

        strlcpy(full_path, dir_path, sizeof(full_path));
        strlcat(full_path, "/", sizeof(full_path));
        strlcat(full_path, SWITCHER_DIR, sizeof(full_path));

        strlcat(full_path, "/", sizeof(full_path));
        strlcat(full_path, SETPROJECTGRP_FILE_NAME, sizeof(full_path));
        retval = stat(full_path, &sbuf);
        if (retval)
            return -1041;
        
        if (sbuf.st_gid != boinc_project_gid)
            return -1042;

        if (sbuf.st_uid != boinc_master_uid)
            return -1043;

        if ((sbuf.st_mode & 07777) != 02500)
            return -1044;

#ifdef __APPLE__
#if 0       // AppStats is deprecated as of version 5.8.15
        strlcat(full_path, "/", sizeof(full_path));
        strlcat(full_path, APP_STATS_FILE_NAME, sizeof(full_path));
        retval = stat(full_path, &sbuf);
        if (retval)
            return -1045;
        
        if (sbuf.st_gid != boinc_master_gid)
            return -1046;

        if (sbuf.st_uid != 0)   // AppStats application must be setuid root
            return -1047;

        if ((sbuf.st_mode & 07777) != 04550)
            return -1048;
#endif
#endif  // __APPLE__

        strlcpy(full_path, dir_path, sizeof(full_path));
        strlcat(full_path, "/", sizeof(full_path));
        strlcat(full_path, SS_CONFIG_FILE, sizeof(full_path));

        retval = stat(full_path, &sbuf);
        if (!retval) {
            if (sbuf.st_uid != boinc_master_uid)
                return -1051;

            if (sbuf.st_gid != boinc_master_gid)
                return -1052;

            if ((sbuf.st_mode & 0777) != 0664)
                return -1053;
        }   // Screensaver config file ss_config.xml exists
        
    }       // if (use_sandbox)
    
    return 0;
}


static int CheckNestedDirectories(char * basepath, int depth, 
                                    int use_sandbox, int isManager, 
                                    char * path_to_error
                                ) {
    int             isDirectory;
    char            full_path[MAXPATHLEN];
    struct stat     sbuf;
    int             retval = 0;
    DIR             *dirp;
    dirent          *dp;
    static int      errShown = 0;

    dirp = opendir(basepath);
    if (dirp == NULL) {
        // Ideally, all project-created subdirectories under project or slot 
        // directoriesshould have read-by-group and execute-by-group permission 
        // bits set, but some don't.  If these permission bits are missing, the 
        // project applications will run OK but we can't access the contents of 
        // the subdirectory to check them.
        strlcpy(full_path, basepath, sizeof(full_path));
        if ((depth > 1) && (errno == EACCES)) {
            return 0;
        } else {
            retval = -1200;
        }
    }

    while (dirp) {              // Skip this if dirp == NULL, else loop until break
        dp = readdir(dirp);
        if (dp == NULL)
            break;                  // End of list
            
        if (dp->d_name[0] == '.')
            continue;               // Ignore names beginning with '.'

        strlcpy(full_path, basepath, sizeof(full_path));
        strlcat(full_path, "/", sizeof(full_path));
        strlcat(full_path, dp->d_name, sizeof(full_path));

        retval = lstat(full_path, &sbuf);
        if (retval)
            break;              // Should never happen

        isDirectory = S_ISDIR(sbuf.st_mode);

        if (!S_ISLNK(sbuf.st_mode)) {   // The system ignores ownership & permissions of symbolic links
            if (depth > 1)  {
                // files and subdirectories created by projects may have owner boinc_master or boinc_project
                if ( (sbuf.st_uid != boinc_master_uid) && (sbuf.st_uid != boinc_project_uid) ) {
                    retval = -1202;
                    break;
                }
            } else {
                // project & slot directories (projects/setiathome.berkeley.edu, slots/0 etc.) 
                // must have owner boinc_master
                if (sbuf.st_uid != boinc_master_uid) {
                    retval = -1202;
                    break;
                }
            }
        
            if (use_sandbox) {
                    if (sbuf.st_gid != boinc_project_gid) {
                        retval = -1201;
                        break;
                    }
                
                if (isDirectory) {
                    if (depth == 1) {
                    // project & slot directories (projects/setiathome.berkeley.edu, slots/0 etc.) 
                    // must be readable & executable by other
                        if ((sbuf.st_mode & 0777) != 0775) {
                            retval = -1203;
                            break;
                        }
#if 0               // We may enforce permissions later for subdirectories written by project applications
                    } else {
                        // subdirectories created by projects may be executable by other or not
                        if ((sbuf.st_mode & 0770) != 0770) {
                            retval = -1203;
                            break;
                        }
#endif
                }
#if 0           // We may enforce permissions later for files written by project applications
                } else {    // ! isDirectory
                    if ((sbuf.st_mode & 0666) != 0660) {
                        retval = -1204;
                        break;
                    }
#endif
                }
            }       // if (use_sandbox)
        }           // if (!S_ISLNK(sbuf.st_mode))
        
        if (isDirectory && !S_ISLNK(sbuf.st_mode)) {
            if (use_sandbox && (depth > 1)) {
#if 0   // No longer check project-created subdirectories under project or slot directories 
        // because we have not told projects these must be readable and executable by group
                if ((! isManager) && (sbuf.st_uid != boinc_master_uid))
#endif
                    continue;       // Client can't check subdirectories owned by boinc_project
            }
            retval = CheckNestedDirectories(full_path, depth + 1, use_sandbox, isManager, path_to_error);
            if (retval)
                break;
        }

    }       // End while (true)

    if (dirp) {
        closedir(dirp);
    }
    
    if (retval && !errShown) {
        fprintf(stderr, "Permissions error %d at %s\n", retval, full_path);
        if (path_to_error) strcpy(path_to_error, full_path);
        errShown = 1;
    }
    return retval;
}


#if (! defined(__WXMAC__) && ! defined(_MAC_INSTALLER))

static char * PersistentFGets(char *buf, size_t buflen, FILE *f) {
    char *p = buf;
    size_t len = buflen;
    size_t datalen = 0;

    *buf = '\0';
    while (datalen < (buflen - 1)) {
        fgets(p, len, f);
        if (feof(f)) break;
        if (ferror(f) && (errno != EINTR)) break;
        if (strchr(buf, '\n')) break;
        datalen = strlen(buf);
        p = buf + datalen;
        len -= datalen;
    }
    return (buf[0] ? buf : NULL);
}


static void GetPathToThisProcess(char* outbuf, size_t maxLen) {
    FILE *f;
    char buf[256], *p, *q;
    pid_t aPID = getpid();

    *outbuf = '\0';
    
    sprintf(buf, "ps -xwo command -p %d", (int)aPID);
    f = popen(buf, "r");
    if (f == NULL)
        return;
    
    PersistentFGets (outbuf, maxLen, f);      // Discard header line
    PersistentFGets (outbuf, maxLen, f);
    pclose(f);

    // Remove trailing newline if present
    p = strchr(outbuf, '\n');
    if (p)
        *p = '\0';
    
    // Strip off any arguments
    p = strstr(outbuf, " -");
    q = p;
    if (p) {
        while (*p == ' ') {
            q = p;
            if (--p < outbuf)
                break;
        }
    }
    
    if (q)
        *q = '\0';
}
#endif


#ifdef __WXMAC__                            // If Mac BOINC Manager
bool IsUserInGroupBM() {
    group               *grp;
    gid_t               rgid;
    char                *userName, *groupMember;
    int                 i;

    grp = getgrnam(REAL_BOINC_MASTER_NAME);
    if (grp) {
        rgid = getgid();
        if (rgid == grp->gr_gid) {
            return true;                // User's primary group is boinc_master
        }

        // On some systems with Automatic Login set, getlogin() returns "root" for a 
        // time after the system is first booted, so we check "USER" environment variable.
        userName = getenv("USER");
        if (userName) {
            for (i=0; ; i++) {          // Step through all users in group boinc_master
                groupMember = grp->gr_mem[i];
                if (groupMember == NULL)
                    return false;       // User is not a member of group boinc_master
                if (strcmp(userName, groupMember) == 0) {
                    return true;        // User is a member of group boinc_master
                }
            }       // for (i)
        }           // if (userName)
    }               // if grp

    return false;
}
#endif  // __WXMAC__
