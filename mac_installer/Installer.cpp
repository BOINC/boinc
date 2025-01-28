// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2022 University of California
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

#define CREATE_LOG 0    /* for general debugging */
#define VERBOSE_TEST 0  /* for debugging callPosixSpawn */

#include <Carbon/Carbon.h>
#include <grp.h>

#include <sys/types.h>	// getpwname, getpwuid, getuid
#include <pwd.h>	// getpwname, getpwuid, getuid
#include <sys/wait.h>	// waitpid
#include <dirent.h>
#include <sys/param.h>  // for MAXPATHLEN
#include <sys/stat.h>

#include "str_util.h"
#include "str_replace.h"
#include "mac_util.h"
#include "translate.h"
#include "file_names.h"
#include "mac_branding.h"

#define boinc_master_user_name "boinc_master"
#define boinc_master_group_name "boinc_master"
#define boinc_project_user_name "boinc_project"
#define boinc_project_group_name "boinc_project"


OSErr Initialize(void);	/* function prototypes */
Boolean IsUserMemberOfGroup(const char *userName, const char *groupName);
Boolean IsRestartNeeded();
static void GetPreferredLanguages();
static void LoadPreferredLanguages();
static long GetOldBrandID(void);
static void ShowMessage(const char *format, ...);
static OSErr QuitAppleEventHandler(const AppleEvent *appleEvt, AppleEvent* reply, UInt32 refcon);
int callPosixSpawn(const char *cmd);
void print_to_log_file(const char *format, ...);
void strip_cr(char *buf);

// We can't use translation because the translation catalogs
// have not yet been installed when this application is run.
#define MAX_LANGUAGES_TO_TRY 5

#define REPORT_ERROR(isError) if (isError) print_to_log_file("BOINC Installer error at line %d", __LINE__);

static char * Catalog_Name = (char *)"BOINC-Setup";
static char Catalogs_Dir[MAXPATHLEN];
static char loginName[256];
static char tempDirName[MAXPATHLEN];

Boolean			gQuitFlag = false;	/* global */

#if 0
CFStringRef     valueRestartRequired = CFSTR("RequiredRestart");
CFStringRef     valueLogoutRequired = CFSTR("RequiredLogout");
CFStringRef     valueNoRestart = CFSTR("NoRestart");
#endif

int main(int argc, char *argv[])
{
    char                    pkgPath[MAXPATHLEN];
    char                    postInstallAppPath[MAXPATHLEN];
    char                    temp[MAXPATHLEN], temp2[MAXPATHLEN];
    char                    brand[64], s[256];
    char                    *p;
    OSStatus                err = noErr;
    Boolean                 restartNeeded = true;
    FILE                    *restartNeededFile;
    FILE                    *f;
    long                    oldBrandID;
    int                     major = 0;
    int                     minor = 0;

    if (!check_branding_arrays(temp, sizeof(temp))) {
        ShowMessage((char *)_("Branding array has too few entries: %s"), temp);
        return -1;
    }

    if (Initialize() != noErr) {
        return 0;
    }

    strncpy(loginName, getenv("USER"), sizeof(loginName)-1);
    if (loginName[0] == '\0') {
        ShowMessage((char *)_("Could not get user login name"));
        return 0;
    }

    snprintf(tempDirName, sizeof(tempDirName), "InstallBOINC-%s", loginName);

    snprintf(temp, sizeof(temp), "/tmp/%s", tempDirName);
    mkdir(temp, 0777);
    chmod(temp, 0777);  // Needed because mkdir sets permissions restricted by umask (022)

    snprintf(temp, sizeof(temp), "rm -dfR /tmp/%s/BOINC_Installer_Errors", tempDirName);
    err = callPosixSpawn(temp);

    snprintf(Catalogs_Dir, sizeof(Catalogs_Dir),
            "/tmp/%s/BOINC_payload/Library/Application Support/BOINC Data/locale/",
            tempDirName);

    // Get the full path to Installer package inside this application's bundle
    getPathToThisApp(pkgPath, sizeof(pkgPath));
    strlcpy(temp, pkgPath, sizeof(temp));

    strlcat(pkgPath, "/Contents/Resources/", sizeof(pkgPath));

    strlcpy(postInstallAppPath, pkgPath, sizeof(postInstallAppPath));
    strlcat(postInstallAppPath, "PostInstall.app", sizeof(postInstallAppPath));

    p = strrchr(temp, '/');         // Point to name of this application (e.g., "BOINC Installer.app")
    if (p == NULL) {
        p = temp - 1;
    } else {
        *p = '\0';
    }

    // Delete any old project auto-attach key file from our temp directory
    snprintf(temp2, sizeof(temp2), "rm -dfR \"/tmp/%s/%s\"", tempDirName, ACCOUNT_DATA_FILENAME);
    err = callPosixSpawn(temp2);
    REPORT_ERROR(err);

    // Write a file containing the project auto-attach key into our temp
    // directory because the BOINC Data directory may not yet exist.
    // PostInstall.app will copy it into the BOINC Data directory later
    snprintf(temp2, sizeof(temp2), "%s/%s", temp, ACCOUNT_DATA_FILENAME);
    if (boinc_file_exists(temp2)) {
        // If the project server put account_data.txt file in the same
        // parent directory as this installer, copy it into our temp directory
        snprintf(temp2, sizeof(temp2), "cp \"%s/%s\" \"/tmp/%s/%s\"", temp, ACCOUNT_DATA_FILENAME, tempDirName, ACCOUNT_DATA_FILENAME);
        err = callPosixSpawn(temp2);
        REPORT_ERROR(err);
    } else {
        // Create an account_data.txt file containing our
        // installer's filename and put it in our temp directory
        snprintf(temp2, sizeof(temp2), "/tmp/%s/%s", tempDirName, ACCOUNT_DATA_FILENAME);
        f = fopen(temp2, "w");
        fputs(p+1, f);
        fclose(f);
    }

    // Write a temp file to tell our PostInstall.app the previous branding, if any
    oldBrandID = GetOldBrandID();
    snprintf(temp, sizeof(temp), "/tmp/%s/OldBranding", tempDirName);
    f = fopen(temp, "w");
    if (!f) {
        REPORT_ERROR(true);
    } else {
        fprintf(f, "BrandId=%ld\n", oldBrandID);
        fclose(f);
    }

    // To allow for branding, assume name of installer package inside bundle corresponds to name of this application
    strlcpy(brand, p+1, sizeof(brand));
    strlcat(pkgPath, p+1, sizeof(pkgPath));
    p = strrchr(pkgPath, ' ');         // Strip off last space character and everything following
    if (p)
        *p = '\0';

    p = strrchr(brand, ' ');         // Strip off last space character and everything following
    if (p)
        *p = '\0';

    strlcat(pkgPath, ".pkg", sizeof(pkgPath));

    // In the unlikely situation that /tmp has files from an earlier attempt to install
    // BOINC by a different user, we won't have permission to delete or overwrite them,
    // so include the current user's name as part of the paths to our temporary files.

    // Expand the installer package
    snprintf(temp, sizeof(temp), "rm -dfR /tmp/%s/BOINC.pkg", tempDirName);
    err = callPosixSpawn(temp);
    REPORT_ERROR(err);
    snprintf(temp, sizeof(temp), "rm -dfR /tmp/%s/expanded_BOINC.pkg", tempDirName);
    err = callPosixSpawn(temp);
    REPORT_ERROR(err);
    snprintf(temp, sizeof(temp), "rm -dfR /tmp/%s/PostInstall.app", tempDirName);
    err = callPosixSpawn(temp);
    REPORT_ERROR(err);
    snprintf(temp, sizeof(temp), "rm -f /tmp/%s/BOINC_preferred_languages", tempDirName);
    err = callPosixSpawn(temp);
    REPORT_ERROR(err);
    snprintf(temp, sizeof(temp), "rm -f /tmp/%s/BOINC_restart_flag", tempDirName);
    err = callPosixSpawn(temp);
    REPORT_ERROR(err);

    sprintf(temp, "cp -fpR \"%s\" /tmp/%s/PostInstall.app", postInstallAppPath, tempDirName);
    err = callPosixSpawn(temp);
    REPORT_ERROR(err);
    sprintf(temp, "pkgutil --expand \"%s\" /tmp/%s/expanded_BOINC.pkg", pkgPath, tempDirName);
    err = callPosixSpawn(temp);
    REPORT_ERROR(err);
    if (err == noErr) {
        GetPreferredLanguages();
    }

    sscanf(Deployment_target, "%i.%i", &major, &minor);
    if (compareOSVersionTo(major, minor) < 0) {
        LoadPreferredLanguages();
        BringAppToFront();
        p = strrchr(brand, ' ');         // Strip off last space character and everything following
        if (p)
            *p = '\0';
        ShowMessage((char *)_("Sorry, this version of %s requires system %s or higher."), brand, Deployment_target);

        snprintf(temp, sizeof(temp), "rm -dfR /tmp/%s/BOINC_payload", tempDirName);
        err = callPosixSpawn(temp);
        REPORT_ERROR(err);
        return -1;
    }

    snprintf(temp, sizeof(temp), "rm -dfR /tmp/%s/BOINC_payload", tempDirName);
    err = callPosixSpawn(temp);
    REPORT_ERROR(err);

    // Remove previous installer package receipt so we can run installer again
    // (affects only older versions of OS X and fixes a bug in those versions)
    // "rm -rf /Library/Receipts/GridRepublic.pkg"
    sprintf(s, "rm -rf \"/Library/Receipts/%s.pkg\"", brand);
    err = callPosixSpawn (s);
    REPORT_ERROR(err);

    restartNeeded = IsRestartNeeded();

    // Write a temp file to tell our PostInstall.app whether restart is needed
    snprintf(temp, sizeof(temp), "/tmp/%s/BOINC_restart_flag", tempDirName);
    restartNeededFile = fopen(temp, "w");
    if (restartNeededFile) {
        fputs(restartNeeded ? "1\n" : "0\n", restartNeededFile);
        fclose(restartNeededFile);
    }

    if (restartNeeded) {
        if (err == noErr) {
            // Change onConclusion="none" to onConclusion="RequireRestart"
            snprintf(temp, sizeof(temp), "sed -i \".bak\" s/onConclusion=\"none\"/onConclusion=\"RequireRestart\"/g /tmp/%s/expanded_BOINC.pkg/Distribution", tempDirName);
            err = callPosixSpawn(temp);
            REPORT_ERROR(err);
        }
        if (err == noErr) {
            snprintf(temp, sizeof(temp), "rm -dfR /tmp/%s/expanded_BOINC.pkg/Distribution.bak", tempDirName);
            err = callPosixSpawn(temp);
            REPORT_ERROR(err);
            // Flatten the installer package
            sprintf(temp, "pkgutil --flatten /tmp/%s/expanded_BOINC.pkg /tmp/%s/%s.pkg", tempDirName, tempDirName, brand);
            err = callPosixSpawn(temp);
            REPORT_ERROR(err);
        }

        if (err == noErr) {
            snprintf(temp, sizeof(temp), "rm -dfR /tmp/%s/expanded_BOINC.pkg", tempDirName);
            err = callPosixSpawn(temp);
            REPORT_ERROR(err);
            sprintf(temp, "open \"/tmp/%s/%s.pkg\"", tempDirName, brand);
            err = callPosixSpawn(temp);
            REPORT_ERROR(err);
            return err;
        }
    }

    snprintf(temp, sizeof(temp), "rm -dfR /tmp/%s/expanded_BOINC.pkg", tempDirName);
    err = callPosixSpawn(temp);
    REPORT_ERROR(err);

    sprintf(temp, "open \"%s\"", pkgPath);
    err = callPosixSpawn(temp);
    REPORT_ERROR(err);

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

    if (compareOSVersionTo(10, 9) >= 0) {
        return false;
    }

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

    if (compareOSVersionTo(10, 5) >= 0) {
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
    if (loginName[0]) {
        if (IsUserMemberOfGroup(loginName, boinc_master_group_name)) {
            return false;   // Logged in user is already a member of group boinc_master
        }
    }
#endif  // SANDBOX

    return true;
}


OSErr Initialize()	/* Initialize some managers */
{
//    InitCursor();

    return AEInstallEventHandler( kCoreEventClass, kAEQuitApplication, NewAEEventHandlerUPP((AEEventHandlerProcPtr)QuitAppleEventHandler), 0, false );
}


// Because language preferences are set on a per-user basis, we
// must get the preferred languages while set to the current
// user, before the Apple Installer switches us to root.
// So we get the preferred languages here and write them to a
// temporary file to be retrieved by our PostInstall app.
static void GetPreferredLanguages() {
    DIR *dirp;
    struct dirent *dp;
    char temp[MAXPATHLEN];
    char searchPath[MAXPATHLEN];
    char savedWD[MAXPATHLEN];
    struct stat sbuf;
    CFMutableArrayRef supportedLanguages;
    CFStringRef aLanguage;
    char shortLanguage[32];
    CFArrayRef preferredLanguages;
    int i, j, k;
    char * language;
    char *uscore;
    FILE *f;

    getcwd(savedWD, sizeof(savedWD));
    snprintf(temp, sizeof(temp), "rm -dfR /tmp/%s/BOINC_payload", tempDirName);
    callPosixSpawn(temp);
    snprintf(temp, sizeof(temp), "/tmp/%s/BOINC_payload", tempDirName);
    mkdir(temp, 0777);
    chmod(temp, 0777);  // Needed because mkdir sets permissions restricted by umask (022)
    chdir(temp);
    // Extract the installer package payload to a temporary location
    snprintf(temp, sizeof(temp), "cpio -i -I /tmp/%s/expanded_BOINC.pkg/BOINC.pkg/Payload", tempDirName);
    callPosixSpawn(temp);
    chdir(savedWD);

    // Create an array of all our supported languages
    supportedLanguages = CFArrayCreateMutable(kCFAllocatorDefault, 100, &kCFTypeArrayCallBacks);

    aLanguage = CFStringCreateWithCString(NULL, "en", kCFStringEncodingMacRoman);
    CFArrayAppendValue(supportedLanguages, aLanguage);
    CFRelease(aLanguage);
    aLanguage = NULL;

    dirp = opendir(Catalogs_Dir);
    if (!dirp) {
        REPORT_ERROR(true);
        goto cleanup;
    }
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

        // If it has a region code ("it_IT") also try without region code ("it")
        // TODO: Find a more general solution
        strlcpy(shortLanguage, dp->d_name, sizeof(shortLanguage));
        uscore = strchr(shortLanguage, '_');
        if (uscore) {
            *uscore = '\0';
            aLanguage = CFStringCreateWithCString(NULL, shortLanguage, kCFStringEncodingMacRoman);
            CFArrayAppendValue(supportedLanguages, aLanguage);
            CFRelease(aLanguage);
            aLanguage = NULL;
        }
    }

    closedir(dirp);

    // Write a temp file to tell our PostInstall.app our preferred languages
    snprintf(temp, sizeof(temp), "/tmp/%s/BOINC_preferred_languages", tempDirName);
    f = fopen(temp, "w");
    if (!f) {
        REPORT_ERROR(true);
        goto cleanup;
    }

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
            if (language == NULL) {
                if (CFStringGetCString(aLanguage, shortLanguage, sizeof(shortLanguage), kCFStringEncodingMacRoman)) {
                    language = shortLanguage;
                }
            }
            if (f && language) {
                fprintf(f, "%s\n", language);
#if CREATE_LOG
                print_to_log_file("Adding language: %s\n", language);
#endif
            }
            // Remove all copies of this language from our list of supported languages
            // so we can get the next preferred language in order of priority
            for (k=CFArrayGetCount(supportedLanguages)-1; k>=0; --k) {
                if (CFStringCompare(aLanguage, (CFStringRef)CFArrayGetValueAtIndex(supportedLanguages, k), 0) == kCFCompareEqualTo) {
                    CFArrayRemoveValueAtIndex(supportedLanguages, k);
                }
            }

            // Since the original strings are English, no
            // further translation is needed for language en.
            if (language) {
                if (!strcmp(language, "en")) {
                    fclose(f);
                    CFRelease(preferredLanguages);
                    preferredLanguages = NULL;
                    goto cleanup;
                }
            }
        }

        CFRelease(preferredLanguages);
        preferredLanguages = NULL;

    }

    if (f) {
        fprintf(f, "en\n");
        fclose(f);
    }

cleanup:
    CFArrayRemoveAllValues(supportedLanguages);
    CFRelease(supportedLanguages);
    supportedLanguages = NULL;
#if CREATE_LOG
    print_to_log_file("Exiting GetPreferredLanguages");
#endif
}


static void LoadPreferredLanguages(){
    FILE *f;
    int i;
    char *p;
    char language[32];
    char temp[MAXPATHLEN];

    BOINCTranslationInit();

    // GetPreferredLanguages() wrote a list of our preferred languages to a temp file
    snprintf(temp, sizeof(temp), "/tmp/%s/BOINC_preferred_languages", tempDirName);
    f = fopen(temp, "r");
    if (!f) {
        REPORT_ERROR(true);
        return;
    }

    for (i=0; i<MAX_LANGUAGES_TO_TRY; ++i) {
        fgets(language, sizeof(language), f);
        if (feof(f)) break;
        language[sizeof(language)-1] = '\0';    // Guarantee a null terminator
        p = strchr(language, '\n');
        if (p) *p = '\0';           // Replace newline with null terminator
        if (language[0]) {
            if (!BOINCTranslationAddCatalog(Catalogs_Dir, language, Catalog_Name)) {
                REPORT_ERROR(true);
                printf("could not load catalog for langage %s\n", language);
            }
        }
    }
    fclose(f);
}


static long GetOldBrandID()
{
    long oldBrandId;

    oldBrandId = 0;   // Default value

    FILE *f = fopen("/Library/Application Support/BOINC Data/Branding", "r");
    if (f) {
        fscanf(f, "BrandId=%ld\n", &oldBrandId);
        fclose(f);
    }
    if ((oldBrandId < 0) || (oldBrandId > (NUMBRANDS-1))) {
        oldBrandId = 0;
    }
    return oldBrandId;
}


static void ShowMessage(const char *format, ...) {
  // CAUTION: vsprintf will produce undesirable results if the string
  // contains a % character that is not a format specification!
  // But CFString is OK!

    va_list                 args;
    char                    s[1024];
    CFOptionFlags           responseFlags;
    CFURLRef                myIconURLRef = NULL;
    CFBundleRef             myBundleRef;

    myBundleRef = CFBundleGetMainBundle();
    if (myBundleRef) {
        myIconURLRef = CFBundleCopyResourceURL(myBundleRef, CFSTR("MacInstaller.icns"), NULL, NULL);
    }

#if 1
    va_start(args, format);
    vsprintf(s, format, args);
    va_end(args);
#else
    strcpy(s, format);
#endif

    // If defaultButton is nil or an empty string, a default localized
    // button title ("OK" in English) is used.

    CFStringRef myString = CFStringCreateWithCString(NULL, s, kCFStringEncodingUTF8);

    BringAppToFront();
    CFUserNotificationDisplayAlert(0.0, kCFUserNotificationPlainAlertLevel,
                myIconURLRef, NULL, NULL, CFSTR(" "), myString,
                NULL, NULL, NULL,
                &responseFlags);

    if (myIconURLRef) CFRelease(myIconURLRef);
    if (myString) CFRelease(myString);
}


static OSErr QuitAppleEventHandler( const AppleEvent *appleEvt, AppleEvent* reply, UInt32 refcon )
{
    gQuitFlag =  true;

    return noErr;
}


#define NOT_IN_TOKEN                0
#define IN_SINGLE_QUOTED_TOKEN      1
#define IN_DOUBLE_QUOTED_TOKEN      2
#define IN_UNQUOTED_TOKEN           3

static int parse_posix_spawn_command_line(char* p, char** argv) {
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
    int argc __attribute__((unused)) = 0;
    char *p;
    pid_t thePid = 0;
    int result = 0;
    int status = 0;
    extern char **environ;

    // Make a copy of cmdline because parse_posix_spawn_command_line modifies it
    strlcpy(command, cmdline, sizeof(command));
    argc = parse_posix_spawn_command_line(const_cast<char*>(command), argv);
    strlcpy(progPath, argv[0], sizeof(progPath));
    strlcpy(progName, argv[0], sizeof(progName));
    p = strrchr(progName, '/');
    if (p) {
        argv[0] = p+1;
    } else {
        argv[0] = progName;
    }

#if VERBOSE_TEST
    print_to_log_file("***********");
    for (int i=0; i<argc; ++i) {
        print_to_log_file("argv[%d]=%s", i, argv[i]);
    }
    print_to_log_file("***********\n");
#endif

    errno = 0;

    result = posix_spawnp(&thePid, progPath, NULL, NULL, argv, environ);
#if VERBOSE_TEST
    print_to_log_file("callPosixSpawn command: %s", cmdline);
    print_to_log_file("callPosixSpawn: posix_spawnp returned %d: %s", result, strerror(result));
#endif
    if (result) {
        return result;
    }
// CAF    int val =
    waitpid(thePid, &status, WUNTRACED);
// CAF        if (val < 0) printf("first waitpid returned %d\n", val);
    if (status != 0) {
#if VERBOSE_TEST
        print_to_log_file("waitpid() returned status=%d", status);
#endif
        result = status;
    } else {
        if (WIFEXITED(status)) {
            result = WEXITSTATUS(status);
            if (result == 1) {
#if VERBOSE_TEST
                print_to_log_file("WEXITSTATUS(status) returned 1, errno=%d: %s", errno, strerror(errno));
#endif
                result = errno;
            }
#if VERBOSE_TEST
            else if (result) {
                print_to_log_file("WEXITSTATUS(status) returned %d", result);
            }
#endif
        }   // end if (WIFEXITED(status)) else
    }       // end if waitpid returned 0 sstaus else

    return result;
}


// For debugging
void print_to_log_file(const char *format, ...) {
    FILE *f;
    va_list args;
    char buf[MAXPATHLEN];
    time_t t;
//    strlcpy(buf, getenv("HOME"), sizeof(buf));
//    strlcat(buf, "/Documents/test_log.txt", sizeof(buf));

    snprintf(buf, sizeof(buf), "/tmp/%s/BOINC_Installer_Errors.txt", tempDirName);
    f = fopen(buf, "a");
    if (!f) return;

//  freopen(buf, "a", stdout);
//  freopen(buf, "a", stderr);

    time(&t);
    strlcpy(buf, asctime(localtime(&t)),sizeof(buf));
    strip_cr(buf);

    fputs(buf, f);
    fputs("   ", f);

    va_start(args, format);
    vfprintf(f, format, args);
    va_end(args);

    fputs("\n", f);
    fflush(f);
    fclose(f);
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
