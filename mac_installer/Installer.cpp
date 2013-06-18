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

/* Installer.cpp */

#define CREATE_LOG 1    /* for debugging */

#include <Carbon/Carbon.h>
#include <grp.h>

#include <unistd.h>	// getlogin
#include <sys/types.h>	// getpwname, getpwuid, getuid
#include <pwd.h>	// getpwname, getpwuid, getuid
#include <sys/wait.h>	// waitpid
#include <dirent.h>
#include <sys/param.h>  // for MAXPATHLEN
#include <sys/stat.h>

#include "str_util.h"
#include "str_replace.h"

#define boinc_master_user_name "boinc_master"
#define boinc_master_group_name "boinc_master"
#define boinc_project_user_name "boinc_project"
#define boinc_project_group_name "boinc_project"

void Initialize(void);	/* function prototypes */
Boolean IsUserMemberOfGroup(const char *userName, const char *groupName);
Boolean IsRestartNeeded();
void GetPreferredLanguages();
OSErr FindProcess (OSType typeToFind, OSType creatorToFind, ProcessSerialNumberPtr processSN);
static OSErr QuitAppleEventHandler(const AppleEvent *appleEvt, AppleEvent* reply, UInt32 refcon);
void print_to_log_file(const char *format, ...);
void strip_cr(char *buf);

// We can't use translation because the translation catalogs
// have not yet been installed when this application is run.
#define _(x) x

#define MAX_LANGUAGES_TO_TRY 5

static char * Catalog_Name = (char *)"BOINC-Setup";
static char * Catalogs_Dir = (char *)"/Library/Application Support/BOINC Data/locale/";

Boolean			gQuitFlag = false;	/* global */

#if 0
CFStringRef     valueRestartRequired = CFSTR("RequiredRestart");
CFStringRef     valueLogoutRequired = CFSTR("RequiredLogout");
CFStringRef     valueNoRestart = CFSTR("NoRestart");
#endif

int main(int argc, char *argv[])
{
    char                    pkgPath[MAXPATHLEN], pkgRestartPath[MAXPATHLEN];
    char                    infoPlistPath[MAXPATHLEN];
    char                    MetaPkgPath[MAXPATHLEN], MetaPkgRestartPath[MAXPATHLEN];
    char                    brand[64], s[256];
    char                    *p;
    ProcessSerialNumber     ourPSN, installerPSN;
    FSRef                   ourFSRef;
    long                    response;
    short                   itemHit;
    pid_t                   installerPID = 0;
    OSStatus                err = noErr;
    struct stat             stat_buf;
    Boolean                 restartNeeded = true;
    FILE                    *restartNeededFile;

    Initialize();

    // Get the full path to Installer package inside this application's bundle
    err = GetCurrentProcess (&ourPSN);
    if (err == noErr)
        err = GetProcessBundleLocation(&ourPSN, &ourFSRef);

    if (err == noErr)
        err = FSRefMakePath (&ourFSRef, (UInt8*)pkgPath, sizeof(pkgPath));

    strlcpy(infoPlistPath, pkgPath, sizeof(infoPlistPath));

    strlcat(pkgPath, "/Contents/Resources/", sizeof(pkgPath));

   // To allow for branding, assume name of installer package inside bundle corresponds to name of this application
    p = strrchr(infoPlistPath, '/');         // Point to name of this application (e.g., "BOINC Installer.app")
    if (p == NULL)
        p = infoPlistPath - 1;
    strlcpy(brand, p+1, sizeof(brand));
    strlcat(pkgPath, p+1, sizeof(pkgPath));
    p = strrchr(pkgPath, ' ');         // Strip off last space character and everything following
    if (p)
        *p = '\0'; 

    p = strstr(brand, " Installer.app");  // Strip off trailing " Installer.app"
    if (p)
        *p = '\0'; 

    strlcpy(MetaPkgPath, pkgPath, sizeof(MetaPkgPath));
    strlcat(MetaPkgPath, "+VirtualBox.mpkg", sizeof(MetaPkgPath));
    
    strlcpy(MetaPkgRestartPath, pkgPath, sizeof(MetaPkgRestartPath));
    strlcat(MetaPkgRestartPath, " + VirtualBox.mpkg", sizeof(MetaPkgRestartPath));
    
    strlcpy(pkgRestartPath, pkgPath, sizeof(MetaPkgPath));
    strlcat(pkgRestartPath, ".mpkg", sizeof(pkgPath));
    
    strlcat(pkgPath, ".pkg", sizeof(pkgPath));
    err = Gestalt(gestaltSystemVersion, &response);
    if (err != noErr)
        return err;

    if (response < 0x1040) {
        ::SetFrontProcess(&ourPSN);
        p = strrchr(brand, ' ');         // Strip off last space character and everything following
        if (p)
            *p = '\0'; 
        s[0] = sprintf(s+1, _("Sorry, this version of %s requires system 10.4 or higher."), brand);
        StandardAlert (kAlertStopAlert, (StringPtr)s, NULL, NULL, &itemHit);

        err = FindProcess ('APPL', 'xins', &installerPSN);
        err = GetProcessPID(&installerPSN , &installerPID);

        if (err == noErr)
            err = kill(installerPID, SIGKILL);

	ExitToShell();
    }

    // Remove previous installer package receipt so we can run installer again
    // (affects only older versions of OS X and fixes a bug in those versions)
    // "rm -rf /Library/Receipts/GridRepublic.pkg"
    sprintf(s, "rm -rf \"/Library/Receipts/%s.pkg\"", brand);
    system (s);

    restartNeeded = IsRestartNeeded();
    
    // Write a temp file to tell our PostInstall.app whether restart is needed
    restartNeededFile = fopen("/tmp/BOINC_restart_flag", "w");
    if (restartNeededFile) {
        fputs(restartNeeded ? "1\n" : "0\n", restartNeededFile);
        fclose(restartNeededFile);
    }
    
    err = Gestalt(gestaltSystemVersion, &response);
    if (err != noErr)
        return err;
    
    if (err == noErr) {
        if ((response < 0x1050) || stat(MetaPkgPath, &stat_buf)) {  // stat() returns zero on success
            sprintf(infoPlistPath, "open \"%s\" &", restartNeeded ? pkgRestartPath : pkgPath);
        } else {
            sprintf(infoPlistPath, "open \"%s\" &", restartNeeded ? MetaPkgRestartPath : MetaPkgPath);
        }
        system(infoPlistPath);
    }

    GetPreferredLanguages();
    
    return err;
}


Boolean IsUserMemberOfGroup(const char *userName, const char *groupName) {
    group               *grp;
    short               i = 0;
    char                *p;

    grp = getgrnam(groupName);
    if (!grp) {
        printf("getgrnam(%s) failed\n", groupName);
        fflush(stdout);
        return false;  // Group not found
    }

    while ((p = grp->gr_mem[i]) != NULL) {  // Step through all users in group admin
        if (strcmp(p, userName) == 0) {
            return true;
        }
        ++i;
    }
    return false;
}


Boolean IsRestartNeeded()
{
    passwd          *pw = NULL;
    group           *grp = NULL;
    gid_t           boinc_master_gid = 0, boinc_project_gid = 0;
    uid_t           boinc_master_uid = 0, boinc_project_uid = 0;
    OSStatus        err = noErr;
    long            response;
    char            loginName[256];
    
    grp = getgrnam(boinc_master_group_name);
    if (grp == NULL)
        return true;       // Group boinc_master does not exist

    boinc_master_gid = grp->gr_gid;
        
    grp = getgrnam(boinc_project_group_name);
    if (grp == NULL)
        return true;       // Group boinc_project does not exist

    boinc_project_gid = grp->gr_gid;

    pw = getpwnam(boinc_master_user_name);
    if (pw == NULL)
        return true;       // User boinc_master does not exist

    boinc_master_uid = pw->pw_uid;

    if (pw->pw_gid != boinc_master_gid)
        return true;       // User boinc_master does not have group boinc_master as its primary group
    
    pw = getpwnam(boinc_project_user_name);
    if (pw == NULL)
        return true;       // User boinc_project does not exist

    boinc_project_uid = pw->pw_uid;
        
    if (pw->pw_gid != boinc_project_gid)
        return true;       // User boinc_project does not have group boinc_project as its primary group

    err = Gestalt(gestaltSystemVersion, &response);
    if ((err == noErr) && (response >= 0x1050)) {
        if (boinc_master_gid < 501)
            return true;       // We will change boinc_master_gid to a value > 501
        if (boinc_project_gid < 501)
            return true;       // We will change boinc_project_gid to a value > 501
        if (boinc_master_uid < 501)
            return true;       // We will change boinc_master_uid to a value > 501
        if (boinc_project_uid < 501)
            return true;       // We will change boinc_project_uid to a value > 501
}
    
    #ifdef SANDBOX
    strncpy(loginName, getenv("USER"), sizeof(loginName)-1);
    if (loginName[0]) {
        if (IsUserMemberOfGroup(loginName, boinc_master_group_name)) {
            return false;   // Logged in user is already a member of group boinc_master
        }
    }
#endif  // SANDBOX

    return true;
}


void Initialize()	/* Initialize some managers */
{
    OSErr	err;
        
//    InitCursor();

    err = AEInstallEventHandler( kCoreEventClass, kAEQuitApplication, NewAEEventHandlerUPP((AEEventHandlerProcPtr)QuitAppleEventHandler), 0, false );
    if (err != noErr)
        ExitToShell();
}


// Because language preferences are set on a per-user basis, we
// must get the preferred languages while set to the current 
// user, before the Apple Installer switches us to root.
// So we get the preferred languages here and write them to a
// temporary file to be retrieved by our PostInstall app.
void GetPreferredLanguages() {
    DIR *dirp;
    struct dirent *dp;
    char searchPath[MAXPATHLEN];
    struct stat sbuf;
    CFMutableArrayRef supportedLanguages;
    CFStringRef aLanguage;
    CFArrayRef preferredLanguages;
    int i, j, k;
    char * language;
    FILE *f;

    // Create an array of all our supported languages
    supportedLanguages = CFArrayCreateMutable(NULL, 100, NULL);
    
    aLanguage = CFStringCreateWithCString(NULL, "en", kCFStringEncodingMacRoman);
    CFArrayAppendValue(supportedLanguages, aLanguage);
    CFRelease(aLanguage);
    aLanguage = NULL;

    dirp = opendir(Catalogs_Dir);
    while (true) {
        dp = readdir(dirp);
        if (dp == NULL)
            break;                  // End of list

        if (dp->d_name[0] == '.')
            continue;               // Ignore names beginning with '.'

        strlcpy(searchPath, Catalogs_Dir, sizeof(searchPath));
        strlcat(searchPath, dp->d_name, sizeof(searchPath));
        strlcat(searchPath, "/", sizeof(searchPath));
        strlcat(searchPath, Catalog_Name, sizeof(searchPath));
        strlcat(searchPath, ".mo", sizeof(searchPath));
        if (stat(searchPath, &sbuf) != 0) continue;
//        printf("Adding %s to supportedLanguages array\n", dp->d_name);
        aLanguage = CFStringCreateWithCString(NULL, dp->d_name, kCFStringEncodingMacRoman);
        CFArrayAppendValue(supportedLanguages, aLanguage);
        CFRelease(aLanguage);
        aLanguage = NULL;
    }
    
    closedir(dirp);

    // Write a temp file to tell our PostInstall.app our preferred languages
    f = fopen("/tmp/BOINC_preferred_languages", "w");

    for (i=0; i<MAX_LANGUAGES_TO_TRY; ++i) {
    
        preferredLanguages = CFBundleCopyLocalizationsForPreferences(supportedLanguages, NULL );
        
#if 0   // For testing
        int c = CFArrayGetCount(preferredLanguages);
        for (k=0; k<c; ++k) {
        CFStringRef s = (CFStringRef)CFArrayGetValueAtIndex(preferredLanguages, k);
            printf("Preferred language %u is %s\n", k, CFStringGetCStringPtr(s, kCFStringEncodingMacRoman));
        }

#endif

        for (j=0; j<CFArrayGetCount(preferredLanguages); ++j) {
            aLanguage = (CFStringRef)CFArrayGetValueAtIndex(preferredLanguages, j);
            language = (char *)CFStringGetCStringPtr(aLanguage, kCFStringEncodingMacRoman);
            if (f) {
                fprintf(f, "%s\n", language);
            }
            
            // Remove this language from our list of supported languages so
            // we can get the next preferred language in order of priority
            for (k=0; k<CFArrayGetCount(supportedLanguages); ++k) {
                if (CFStringCompare(aLanguage, (CFStringRef)CFArrayGetValueAtIndex(supportedLanguages, k), 0) == kCFCompareEqualTo) {
                    CFArrayRemoveValueAtIndex(supportedLanguages, k);
                    break;
                }
            }

            // Since the original strings are English, no 
            // further translation is needed for language en.
            if (!strcmp(language, "en")) {
                fclose(f);
                CFRelease(preferredLanguages);
                preferredLanguages = NULL;
                CFArrayRemoveAllValues(supportedLanguages);
                CFRelease(supportedLanguages);
                supportedLanguages = NULL;
                return;
            }
        }
        
        CFRelease(preferredLanguages);
        preferredLanguages = NULL;

    }

    fclose(f);

    CFArrayRemoveAllValues(supportedLanguages);
    CFRelease(supportedLanguages);
    supportedLanguages = NULL;
}


// ---------------------------------------------------------------------------
/* This runs through the process list looking for the indicated application */
/*  Searches for process by file type and signature (creator code)          */
// ---------------------------------------------------------------------------
OSErr FindProcess (OSType typeToFind, OSType creatorToFind, ProcessSerialNumberPtr processSN)
{
    ProcessInfoRec tempInfo;
    FSSpec procSpec;
    Str31 processName;
    OSErr myErr = noErr;
    /* null out the PSN so we're starting at the beginning of the list */
    processSN->lowLongOfPSN = kNoProcess;
    processSN->highLongOfPSN = kNoProcess;
    /* initialize the process information record */
    tempInfo.processInfoLength = sizeof(ProcessInfoRec);
    tempInfo.processName = processName;
    tempInfo.processAppSpec = &procSpec;
    /* loop through all the processes until we */
    /* 1) find the process we want */
    /* 2) error out because of some reason (usually, no more processes) */
    do {
        myErr = GetNextProcess(processSN);
        if (myErr == noErr)
            GetProcessInformation(processSN, &tempInfo);
    }
            while ((tempInfo.processSignature != creatorToFind || tempInfo.processType != typeToFind) &&
                   myErr == noErr);
    return(myErr);
}


static OSErr QuitAppleEventHandler( const AppleEvent *appleEvt, AppleEvent* reply, UInt32 refcon )
{
    gQuitFlag =  true;
    
    return noErr;
}


// For debugging
void print_to_log_file(const char *format, ...) {
#if CREATE_LOG
    FILE *f;
    va_list args;
    char buf[256];
    time_t t;
    safe_strcpy(buf, getenv("HOME"));
    safe_strcat(buf, "/Documents/test_log.txt");
    f = fopen(buf, "a");
    if (!f) return;

//  freopen(buf, "a", stdout);
//  freopen(buf, "a", stderr);

    time(&t);
    safe_strcpy(buf, asctime(localtime(&t)));
    strip_cr(buf);

    fputs(buf, f);
    fputs("   ", f);

    va_start(args, format);
    vfprintf(f, format, args);
    va_end(args);
    
    fputs("\n", f);
    fflush(f);
    fclose(f);
#endif
}

#if CREATE_LOG
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
#endif	// CREATE_LOG
