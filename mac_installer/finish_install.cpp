// This file is part of BOINC.
// http://boinc.berkeley.edu
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
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

//
//  Finish_install.cpp
//  boinc_Finish_Install

// Usage: boinc_Finish_Install [-d | -m] | -i brandID | -a brandID]
//
// * Deletes Login Items of all possible branded and unbranded BOINC Managers for current user.
// * If first argument is -d then also kills the application specified by the second argument.
// * If first argument is the name of a branded or unbranded BOINC Manager, adds it as a Login
//   Item for the current user and launches it.
//
// TODO: Do we ned to code sign this app?
//

#define VERBOSE_TEST 0  /* for debugging callPosixSpawn */
#if VERBOSE_TEST
#define CREATE_LOG 1    /* for debugging */
#else
#define CREATE_LOG 0    /* for debugging */
#endif

#include <Carbon/Carbon.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>    // waitpid
#include <sys/param.h>  // for MAXPATHLEN
#include <sys/stat.h>   // for chmod
#include <string.h>
#include <ctype.h>
#include <cerrno>
#include <sys/time.h>
#include <stdarg.h>
#include <unistd.h>
#include <pwd.h>    // getpwname, getpwuid, getuid
#include <spawn.h>
#include <dirent.h>

#include "mac_branding.h"
#include "translate.h"
#include "mac_util.h"

#define MAX_LANGUAGES_TO_TRY 5

// IMPORTANT: The definition of COPY_FINISH_INSTALL_TO_USER_DIRECTORY
// must match the one in PostInstall.cpp
#define COPY_FINISH_INSTALL_TO_USER_DIRECTORY false


static char * Catalog_Name = (char *)"BOINC-Setup";
static char * Catalogs_Dir = "/Library/Application Support/BOINC Data/locale/";

static int callPosixSpawn(const char *cmd);
static Boolean MakeLaunchManagerLaunchAgent(long brandID, passwd *pw);
static void FixLaunchServicesDataBase(int brandId, bool isUninstall);
static Boolean IsUserActive(const char *userName);
void MaybeSetScreenSaver(int brandId);
static char * PersistentFGets(char *buf, size_t buflen, FILE *f);
static Boolean ShowMessage(Boolean askYesNo, const char *format, ...);
static void GetAndLoadPreferredLanguages();
static void print_to_log_file(const char *format, ...);

int main(int argc, const char * argv[]) {
    int                     i, err;
    char                    cmd[2048];
    char                    scriptName[1024];
    char                    *userName;
    passwd                  *pw;
    bool                    isUninstall = false;
    int                     iBrandId = 0;
    bool                    calledFromInstaller = false;
    bool                    createdByUninstaller = false;
    bool                    calledFromManager = false;

    // Wait until we are the active login (in case of fast user switching)
    userName = getenv("USER");
    while (!IsUserActive(userName)) {
        sleep(1);
    }

    pw = getpwuid(getuid());

    for (i=1; i<argc; i++) {
        if (strcmp(argv[i], "-d") == 0) {
            isUninstall = true;
        } else if (strcmp(argv[i], "-a") == 0) {
            if (i + 1 < argc) {
                iBrandId = atoi(argv[++i]);
            }
        } else if (strcmp(argv[i], "-i") == 0) {
            if (i + 1 < argc) {
                iBrandId = atoi(argv[++i]);
            }
            calledFromInstaller = true;
        } else if (strcmp(argv[i], "-m") == 0) {
            calledFromManager = true;
        } else if (strcmp(argv[i], "-u") == 0) {
            isUninstall = true;
            createdByUninstaller = true;
        }
    }   // end for (i=i; i<argc; i+=2)

    if (isUninstall) {
        // If this user was previously authorized to run the Manager, the Login Item
        // may have launched the Manager before this app deleted that Login Item. To
        // guard against this, we kill the Manager (for this user only) if it is running.
        //
        snprintf(cmd, sizeof(cmd), "killall -u %d -9 \"%s\"", getuid(), appName[iBrandId]);
        err = callPosixSpawn(cmd);
        if (err) {
        }

        if (compareOSVersionTo(10, 13) >= 0) {
            snprintf(cmd, sizeof(cmd), "launchctl unload \"/Users/%s/Library/LaunchAgents/edu.berkeley.launchboincmanager.plist\"", pw->pw_name);
            err = callPosixSpawn(cmd);
            if (err) {
            }
            sprintf(cmd, "rm -f \"/Users/%s/Library/LaunchAgents/edu.berkeley.launchboincmanager.plist\"", pw->pw_name);
            callPosixSpawn (cmd);
            if (err) {
             }
        }

    } else {
        bool success = MakeLaunchManagerLaunchAgent(iBrandId, pw);
        if (!success) {
        }
        if (compareOSVersionTo(26, 0) >= 0) {
            GetAndLoadPreferredLanguages();
            MaybeSetScreenSaver(iBrandId);
        }

        if (compareOSVersionTo(10, 13) >= 0) {
            snprintf(cmd, sizeof(cmd), "launchctl unload \"/Users/%s/Library/LaunchAgents/edu.berkeley.launchboincmanager.plist\"", pw->pw_name);
            err = callPosixSpawn(cmd);
            if (err) {
            }

            if (! (calledFromInstaller || calledFromManager)) {
                snprintf(cmd, sizeof(cmd), "launchctl load \"/Users/%s/Library/LaunchAgents/edu.berkeley.launchboincmanager.plist\"", pw->pw_name);
                err = callPosixSpawn(cmd);
                if (err) {
                }
            }

        } else {
            snprintf(cmd, sizeof(cmd), "open -jg \"%s\"", appPath[iBrandId]);
            err = callPosixSpawn(cmd);
            if (err) {
            }
        }
    }

    if (! calledFromManager) {
        FixLaunchServicesDataBase(iBrandId, isUninstall);
    }

    if (! (calledFromInstaller || calledFromManager)) {
        snprintf(cmd, sizeof(cmd), "rm -f \"/Users/%s/Library/LaunchAgents/edu.berkeley.boinc.plist\"", pw->pw_name);
        callPosixSpawn(cmd);

        // We can't delete ourselves while we are running,
        // so launch a shell script to do it after we exit.
        sprintf(scriptName, "/tmp/%s_Finish_%s_%s", brandName[iBrandId], isUninstall ? "Uninstall" : "Install", pw->pw_name);
        FILE* f = fopen(scriptName, "w");
        fprintf(f, "#!/bin/bash\n\n");
        fprintf(f, "sleep 3\n");
        if (isUninstall) {
            // Delete per-user BOINC Manager and screensaver files, including this executable
            fprintf(f, "rm -fR \"/Users/%s/Library/Application Support/BOINC\"\n", pw->pw_name);
        } else {
#if ! COPY_FINISH_INSTALL_TO_USER_DIRECTORY
            // The installer no longer put a copy of %s_Finish_Install in the user's folder;
            // all users now call the one in the BOINC Data folder
            // But the uninstaller stil puts a copy of %s_Finish_Uninstall in the user's
            // folder (in case the user might have already deleted the BOINC Data directory.)
            // So we don't want to delete ourselves unless we were created by the uninstaller.
            // The uninstaller set up the launchagent to pass us a "u" to tell us self-delete.
            if (createdByUninstaller)
#endif
            {
                // Delete only this executable
                fprintf(f, "rm -fR \"/Users/%s/Library/Application Support/BOINC/%s_Finish_Install.app\"", pw->pw_name, brandName[iBrandId]);
            }
        }
        fclose(f);
        sprintf(cmd, "sh \"%s\"", scriptName);
        callPosixSpawn (cmd);
    }

    return 0;
}


static Boolean MakeLaunchManagerLaunchAgent(long brandID, passwd *pw)
{
    struct stat             sbuf;
    char                    s[2048];

    // Create a LaunchAgent for the specified user to autostart BOINC Manager on login, replacing
    // any LaunchAgent created previously (such as by installing a differently branded BOINC.)

    // Create LaunchAgents directory for this user if it does not yet exist
    snprintf(s, sizeof(s), "/Users/%s/Library/LaunchAgents", pw->pw_name);
    if (stat(s, &sbuf) != 0) {
        mkdir(s, 0755);
        chown(s, pw->pw_uid, pw->pw_gid);
    }

    snprintf(s, sizeof(s), "/Users/%s/Library/LaunchAgents/edu.berkeley.launchboincmanager.plist", pw->pw_name);
    FILE* f = fopen(s, "w");
    if (!f) return false;
    fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(f, "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n");
    fprintf(f, "<plist version=\"1.0\">\n");
    fprintf(f, "<dict>\n");
    fprintf(f, "\t<key>Label</key>\n");
    fprintf(f, "\t<string>edu.berkeley.launchBOINCManager</string>\n");
    fprintf(f, "\t<key>ProgramArguments</key>\n");
    fprintf(f, "\t<array>\n");
    fprintf(f, "\t\t<string>/Library/Application Support/%s.app/Contents/MacOS/%s</string>\n", appName[brandID], appName[brandID]);
    fprintf(f, "\t\t<string>--autostart</string>\n");
    fprintf(f, "\t</array>\n");
    if (compareOSVersionTo(13, 0) >= 0) {
        fprintf(f, "\t<key>AssociatedBundleIdentifiers</key>\n");
        fprintf(f, "\t<string>edu.berkeley.boinc</string>\n");
    }
    fprintf(f, "\t<key>RunAtLoad</key>\n");
    fprintf(f, "\t<true/>\n");
    fprintf(f, "</dict>\n");
    fprintf(f, "</plist>\n");
    fclose(f);

    chmod(s, 0644);
    chown(s, pw->pw_uid, pw->pw_gid);

    return true;
}

// If there are other copies of BOINC Manager with different branding
// on the system, Notifications may display the icon for the wrong
// branding, due to the Launch Services database having one of the
// other copies of BOINC Manager as the first entry. Each user has
// their own copy of the Launch Services database, so this must be
// done for each user.
//
// This probably will happen only on BOINC development systems where
// Xcode has generated copies of BOINC Manager.
static void FixLaunchServicesDataBase(int brandID, bool isUninstall) {
    char boincPath[MAXPATHLEN];
    char cmd[MAXPATHLEN+250];
    long i, n;
    CFArrayRef appRefs = NULL;
    OSStatus err;

    CFStringRef bundleID = CFSTR("edu.berkeley.boinc");

    // LSCopyApplicationURLsForBundleIdentifier is not available before OS 10.10,
    // but this app is used only for OS 10.13 and later
    appRefs = LSCopyApplicationURLsForBundleIdentifier(bundleID, NULL);
    if (appRefs == NULL) {
        print_to_log_file("Call to LSCopyApplicationURLsForBundleIdentifier returned NULL");
        goto registerOurApp;
    }
    n = CFArrayGetCount(appRefs);   // Returns all results at once, in database order
    print_to_log_file("LSCopyApplicationURLsForBundleIdentifier returned %ld results", n);

    for (i=0; i<n; ++i) {     // Prevent infinite loop
        CFURLRef appURL = (CFURLRef)CFArrayGetValueAtIndex(appRefs, i);
        boincPath[0] = '\0';
        if (appURL) {
            CFRetain(appURL);
            CFStringRef CFPath = CFURLCopyFileSystemPath(appURL, kCFURLPOSIXPathStyle);
            CFStringGetCString(CFPath, boincPath, sizeof(boincPath), kCFStringEncodingUTF8);
            if (CFPath) CFRelease(CFPath);
            CFRelease(appURL);
            appURL = NULL;
        }
        if (! isUninstall) {
            if (strncmp(boincPath, appPath[brandID], sizeof(boincPath)) == 0) {
                print_to_log_file("**** Keeping %s", boincPath);
                if (appRefs) CFRelease(appRefs);
                return;     // Our (possibly branded) BOINC Manager app is now at top of database
            }
        }
        print_to_log_file("Unregistering %3ld: %s", i, boincPath);
        // Remove this entry from the Launch Services database
        sprintf(cmd, "/System/Library/Frameworks/CoreServices.framework/Frameworks/LaunchServices.framework/Versions/Current/Support/lsregister -u \"%s\"", boincPath);
        err = callPosixSpawn(cmd);
        if (err) {
            print_to_log_file("*** lsregister -u call returned error %d for %s", err, boincPath);
        }
    }

registerOurApp:
    if (appRefs) CFRelease(appRefs);

    if (isUninstall) return;

    // We have exhausted the Launch Services database without finding our
    // (possibly branded) BOINC Manager app, so add it to the dataabase
    print_to_log_file("%s was not found in Launch Services database; registering it now", appPath[brandID]);
    sprintf(cmd, "/System/Library/Frameworks/CoreServices.framework/Frameworks/LaunchServices.framework/Versions/Current/Support/lsregister \"%s\"", appPath[brandID]);
    err = callPosixSpawn(cmd);
    if (err) {
        print_to_log_file("*** lsregister call returned error %d for %s", err, appPath[brandID]);
        fflush(stdout);
    }
}


static char * PersistentFGets(char *buf, size_t buflen, FILE *f) {
    char *p = buf;
    int len = (int)buflen;
    size_t datalen = 0;
    memset(buf, 0, buflen);
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


static Boolean IsUserActive(const char *userName){
    char s[1024];

    FILE *f = popen("ls -l /dev/console", "r");
    if (f) {
        while (PersistentFGets(s, sizeof(s), f) != NULL) {
            if (strstr(s, userName)) {
                pclose (f);
                return true;
            }
        }
        pclose (f);
    }
    return false;
}


// It would be nice to check whether our screen saver is already set for
// this user so we can ask wethr to set it only if necessary, but that
// triggrs a scary warning asking for permission to let System Events
// administer the Mac, so we don't. (If the user answers yes to setting
// the screensaver, then the code which sets the screensaver will show
// that warning, but this seems less scary than having it pop up for no
// apparent reason.)
void MaybeSetScreenSaver(int brandId){
    char                buf[MAXPATHLEN];
    char                mySaverName[1024];

    if ((ShowMessage(true,
        (char *)_("Do you want to set %s as your screensaver?"),
                    brandName[brandId], brandName[brandId])) == false) {
        return;
    }

    CFBundleRef myBundle = NULL;
    mySaverName[0] = '\0';
    snprintf(buf, sizeof(buf), "/Library/Screen Savers/%s.saver", saverName[brandId]);
    CFStringRef cfpath = CFStringCreateWithCString(NULL, buf, kCFStringEncodingUTF8);
    if (cfpath) {
        CFURLRef bundleURL = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, cfpath, kCFURLPOSIXPathStyle, true);
        if (bundleURL) {
            myBundle = CFBundleCreate(kCFAllocatorDefault, bundleURL);
            CFRelease(bundleURL);
        }
        CFRelease(cfpath);
    }

    if (myBundle) {
        CFStringRef bundleNameCF = (CFStringRef)CFBundleGetValueForInfoDictionaryKey(myBundle, CFSTR("CFBundleName"));
        if (bundleNameCF) {
            strncpy(mySaverName, CFStringGetCStringPtr(bundleNameCF, kCFStringEncodingUTF8), sizeof(mySaverName));
        }
        if(mySaverName[0] == '\0') {
            strncpy(mySaverName, saverName[brandId], sizeof(mySaverName));
        }

        // The AppleEvent call doesn't work unless we have opened the Sysem Preferences
        // Wallpaper pane, even though that's not necessary if we do the osascript from
        // Terminal.app or run the Applescript from the Script Editor or  from an
        // Applescript application created by the Script Editor.
        //
        // Use -n flag to open a new instance of the System Settings even if one is
        // already running. We can then use the returned PID to quit only that instance.
        callPosixSpawn("open -gj \"x-apple.systempreferences:com.apple.Wallpaper-Settings.extension\"");

        sleep(1); // This delay seems to be needed, probably to give Wallpaper pane time to open

        snprintf(buf, sizeof(buf), "osascript -e 'tell application \"System Events\" to set current screen saver to screen saver \"%s\"'", mySaverName);
        callPosixSpawn(buf);

        // TODO: find a way to kill only the new hidden instance of System Settigs that we opened
        callPosixSpawn("killall \"System Settings\"");

        CFRelease(myBundle);
    }
}


static void GetAndLoadPreferredLanguages() {
    DIR *dirp;
    struct dirent *dp;
    char searchPath[MAXPATHLEN];
    struct stat sbuf;
    CFMutableArrayRef supportedLanguages;
    CFStringRef aLanguage;
    char shortLanguage[32];
    CFArrayRef preferredLanguages;
    int i, j, k;
    char * language;
    char *uscore;

    // Create an array of all our supported languages
    supportedLanguages = CFArrayCreateMutable(kCFAllocatorDefault, 100, &kCFTypeArrayCallBacks);

    aLanguage = CFStringCreateWithCString(NULL, "en", kCFStringEncodingMacRoman);
    CFArrayAppendValue(supportedLanguages, aLanguage);
    CFRelease(aLanguage);
    aLanguage = NULL;

    dirp = opendir(Catalogs_Dir);
    if (!dirp) {
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
            if (language) {
#if CREATE_LOG
                print_to_log_file("Adding language: %s\n", language);
#endif
                if (!BOINCTranslationAddCatalog(Catalogs_Dir, language, Catalog_Name)) {
                    printf("could not load catalog for langage %s\n", language);
                }
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
                    CFRelease(preferredLanguages);
                    preferredLanguages = NULL;
                    goto cleanup;
                }
            }
        }

        CFRelease(preferredLanguages);
        preferredLanguages = NULL;
    }

    if (!BOINCTranslationAddCatalog(Catalogs_Dir, "en", Catalog_Name)) {
        printf("could not load catalog for langage en\n");
    }

cleanup:
    CFArrayRemoveAllValues(supportedLanguages);
    CFRelease(supportedLanguages);
    supportedLanguages = NULL;
#if CREATE_LOG
    print_to_log_file("Exiting GetAndLoadPreferredLanguages");
#endif
}


static Boolean ShowMessage(Boolean askYesNo, const char *format, ...) {
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

#if 0
    enum {
   kCFUserNotificationDefaultResponse = 0,
   kCFUserNotificationAlternateResponse = 1,
   kCFUserNotificationOtherResponse = 2,
   kCFUserNotificationCancelResponse = 3
};
#endif

    CFStringRef myString = CFStringCreateWithCString(NULL, s, kCFStringEncodingUTF8);
    CFStringRef yes = CFStringCreateWithCString(NULL, (char*)_((char*)"Yes"), kCFStringEncodingUTF8);
    CFStringRef no = CFStringCreateWithCString(NULL, (char*)_((char*)"No"), kCFStringEncodingUTF8);

//    BringAppToFront();
    SInt32 retval = CFUserNotificationDisplayAlert(0.0, kCFUserNotificationPlainAlertLevel,
                myIconURLRef, NULL, NULL, CFSTR(" "), myString,
                askYesNo ? yes : NULL, askYesNo ? no : NULL, NULL,
                &responseFlags);


    if (myIconURLRef) CFRelease(myIconURLRef);
    if (myString) CFRelease(myString);
    if (yes) CFRelease(yes);
    if (no) CFRelease(no);

    if (retval) return false;
    return (responseFlags == kCFUserNotificationDefaultResponse);
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


static int callPosixSpawn(const char *cmdline) {
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
#endif    // CREATE_LOG

static void print_to_log_file(const char *format, ...) {
#if CREATE_LOG
    va_list args;
    char buf[256];
    time_t t;
    sprintf(buf, "/Users/Shared/test_log_finish_install.txt");
    FILE *f;
    f = fopen(buf, "a");
    if (!f) return;

    // File may be owned by various users, so make it world readable & writable
    chmod(buf, 0666);

    time(&t);
    strlcpy(buf, asctime(localtime(&t)), sizeof(buf));

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
