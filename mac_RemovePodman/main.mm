//
//  main.m
//  RemovePodman
//
//  Created by Charlie Fenton on 1/26/26.
//

#import <Cocoa/Cocoa.h>
#include <sys/stat.h>
#include <pwd.h>	// getpwnam
#include <dirent.h>
#import <ServiceManagement/ServiceManagement.h>
#import <Security/Authorization.h>

static Boolean ShowMessage(Boolean allowCancel, Boolean continueButton, Boolean yesNoButtons, const char *format, ...);

// While the official Podman installer puts the Podman executable at
// "/opt/podman/bin/podman", other installation methods (e.g. brew) might not
static void find_podman_path(char *path, size_t len) {
    // Mac executables get a very limited PATH environment variable, so we must get the
    // PATH variable used by Terminal and search there for the path to podman
    struct stat buf;
    char allpaths[2048];
    char cmd[2048];

    path[0] = '\0';
    FILE *f = popen("a=`/usr/libexec/path_helper`;b=${a%\\\"*}\\\";echo ${b}", "r");
    if (f) {
        fgets(allpaths, sizeof(allpaths), f);
        int err = pclose(f);
        if (err) {
            f = NULL;
        } else {
            char* p = strstr(allpaths, "\n");
            if (p) *p = '\0'; // Remove the newline character

            snprintf(cmd, sizeof(cmd), "env %s which podman", allpaths);
            f = popen(cmd, "r");
        }
    }
    if (f) {
        fgets(path, (int)len, f);
        pclose(f);
        char* p = strstr(path, "\n");
        if (p) *p = '\0'; // Remove the newline character
        if (path[0] != '\0') {
            if (stat(path, &buf) == 0) return;
        }
    }

    // If we couldn't get it from that file, use default when installed using Podman installer
    strlcpy(path, "/opt/podman/bin/podman", len);
    if (stat(path, &buf) == 0) return;

    // If we couldn't get it from that file, use default when installed by Homebrew
#ifdef __arm64__
    strlcpy(path, "/opt/homebrew/bin/podman", len);
#else
    strlcpy(path, "/usr/local/bin/podman", len);
#endif
    if (stat(path, &buf) == 0) return;
    path[0] = '\0'; // Failed to find path to Podman
    return;
}

int DoCommand(char *cmd) {
    passwd *pw;
    char buf[2048];
    FILE* f;

    uid_t euid = geteuid();
    pw = getpwuid(euid);
    if (!pw) {
        printf("Can't get user name from uid %d", euid);
        return -1;
    }

    printf("\nUser %s: About to do %s\n", pw->pw_name, cmd);

    f = popen(cmd, "r");
    if (!f) {
        printf("User %s: popen \"%s\" returned null", pw->pw_name, cmd);
        return -1;
    }
    while (fgets(buf, sizeof(buf), f)) {
        puts(buf);
    }
    // Note: we ignore errors from the actual command (its exit status)
    pclose(f);

    return 0;
}


// After running this app, we recommend restarting the computer to
// clean up any <defunct> podman processes and to ensure that the
// QEMU or com.apple.Virtualization.VirtualMachiine has exited.
//
int doRemovePodman(char *podmanPath) {
    passwd *pw;
    uid_t boinc_project_uid = 0;
    uid_t euid = geteuid();
    char cmd[2048];
    int err;
    DIR *dirp;
    dirent *dp;
    extern int errno;

    pw = getpwnam("boinc_project");
    if (pw) {
        seteuid(pw->pw_uid);
    }

    if (pw) boinc_project_uid = pw->pw_uid;
    if (geteuid() != boinc_project_uid) {
        fprintf(stderr, "\nERROR: This program must be run as effective user boinc_project (euid %d)\n", boinc_project_uid);
        fprintf(stderr, "Effective user id is %d\n", euid);
        return 1;
    }

#if 0   // For debugging
    printf("uid=%d euid=%d\n", getuid(), geteuid());
    DoCommand("whoami");
#endif

    err =   chdir("/Library/Application Support/BOINC Data");
    if (err) {
        perror("Could not chdir(\"/Library/Application Support/BOINC Data\"");
        return err;
    }

    err = DoCommand((char *)"killall -KILL podman");
    if (err) return err;

    snprintf(cmd, sizeof(cmd), "./Run_Podman \"%s\" machine stop\n",  podmanPath);
    err = DoCommand(cmd);
    if (err) return err;

    snprintf(cmd, sizeof(cmd), "./Run_Podman \"%s\" machine rm --force",  podmanPath);
    err = DoCommand(cmd);
    if (err) return err;

    seteuid(0); // Set effective user back to root

    // Step through all visible users
    dirp = opendir("/Users");
    if (dirp == NULL) {
        puts("opendir(\"/Users\") failed");
        return 1;
    }

    while (dirp) {              // Skip this if dirp == NULL, else loop until break
        dp = readdir(dirp);
        if (dp == NULL) {
            break;                  // End of list
        }

        if (dp->d_name[0] == '.') {
            continue;               // Ignore names beginning with '.'
        }

        if (!strcmp(dp->d_name, "Shared")) {
            continue;
        }

        if (!strcmp(dp->d_name, "boinc_project")) {
            continue;
        }

        if (!strcmp(dp->d_name, "boinc_master")) {
            continue;
        }

        pw = getpwnam(dp->d_name);
        if (!pw) {
            continue;
        }
        seteuid(pw->pw_uid);

        err = DoCommand((char *)"killall -KILL podman");
        if (err) goto bail;

        snprintf(cmd, sizeof(cmd), "\"%s\" machine stop\n",  podmanPath);
        err = DoCommand(cmd);
        if (err) goto bail;

        snprintf(cmd, sizeof(cmd), "\"%s\" machine rm --force\n",  podmanPath);
        err = DoCommand(cmd);
        if (err) goto bail;

        err = chdir("/Users");
        if (err) {
            perror("Could not chdir(\"/Users\"");
        if (err) goto bail;
        }

        err = chdir(dp->d_name);
        if (err) {
            snprintf(cmd, sizeof(cmd), "Could not chdir(\"/Users/%s\"",  dp->d_name);
            perror(cmd);
        if (err) goto bail;
        }

        snprintf(cmd, sizeof(cmd), "rm -Rf .local/share/containers");
        err = DoCommand(cmd);
        if (err) goto bail;

        snprintf(cmd, sizeof(cmd), "rm -Rf .config/containers");
        err = DoCommand(cmd);
        if (err) goto bail;

        snprintf(cmd, sizeof(cmd), "rm -Rf .ssh/*podman*");
        err = DoCommand(cmd);
        if (err) goto bail;

bail:
        seteuid(0); // Set effective user back to root
    }
    if (dirp) {
        closedir(dirp);
    }

    // Remove contents of "BOINC podman" directory but not the directory itself
    err = DoCommand((char *)"rm -R \"/Library/Application Support/BOINC podman/\"*");
    if (err) return err;

    // The following commands remove podman
    err = DoCommand((char *)"rm -R /opt/podman");
    if (err) return err;

    err = DoCommand((char *)"rm /opt/homebrew/bin/podman");
    if (err) return err;

    err = DoCommand((char *)"rm /opt/homebrew/bin/podman*");
    if (err) return err;

    err = DoCommand((char *)"rm -Rf /opt/homebrew/Cellar/podman");
    if (err) return err;

    err = DoCommand((char *)"rm -Rf /private/etc/paths.d/podman-pkg");
    if (err) return err;

    err = DoCommand((char *)"rm -Rf /usr/local/podman");
    if (err) return err;

    err = DoCommand((char *)"rm -Rf /Library/LaunchDaemons/*podman*");
    if (err) return err;

    puts("\nEnd of attempted removal of Podman\n");
    return 0;
}

int main(int argc, const char * argv[]) {
    char                        podmanPath[2048];
    char                        pathToSelf[MAXPATHLEN];
    char                        appName[MAXPATHLEN];
    char                        *p;
    char *                      myArgv[2];
    char                        logPath[MAXPATHLEN];
    Boolean                     isPrivileged = false;
    int                         err;

   // Determine whether this is the initial launch or the relaunch with privileges
    for (int i=0; i<argc; ++i) {
        if (strcmp(argv[i], "--privileged") == 0) {
            isPrivileged = true;
        }
    }

    if (! isPrivileged) {
        if (! ShowMessage(true, true, false, "WARNING: This will cancel any BOINC and non-BOINC Podman tasks in progress"
                " and will completely remove PODMAN from your computer.\n\n"
                "Do you wish to continue?"
                )) {
                return 0;
        }
    }

    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = [paths objectAtIndex:0];
    strlcpy(logPath, [documentsDirectory UTF8String], sizeof(logPath));
    strlcat(logPath, "/RemovePodmanLog.txt", sizeof(logPath));

    FILE *stdout_file = freopen(logPath, "a", stdout);
    if (stdout_file) {
        setbuf(stdout_file, 0);
    }
    FILE *stderr_file = freopen(logPath, "a", stderr);
    if (stderr_file) {
        setbuf(stderr_file, 0);
    }

    find_podman_path(podmanPath, sizeof(podmanPath));
    if (podmanPath[0] == '\0') {
        puts("Failed to find path to podman");
        return 1;
    }
    printf("\nPodman is at %s\n\n", podmanPath);


    if (isPrivileged) {
        setuid(0);  // This is permitted bcause euid == 0
        return doRemovePodman(podmanPath);
    }

    puts("\nStarting attempt to remove Podman\n");
    chmod(logPath, 0666);

    pathToSelf[0] = '\0';
    // Get the full path to our executable inside this application's bundle
        // Get the app's main bundle
    NSBundle *main = [NSBundle mainBundle];
    NSString *thePath = [main bundlePath];
    strlcpy(pathToSelf, [thePath UTF8String], sizeof(pathToSelf));
    if (!pathToSelf[0]) {
        fprintf(stderr, "Couldn't get path to self.");
        return -1;
    }
    p = strrchr(pathToSelf, '/');         // Assume name of executable inside bundle is same as name of bundle
    if (p == NULL)
        p = pathToSelf - 1;
    strlcpy(appName, p+1, sizeof(appName));
    p = strrchr(appName, '.');         // Strip off bundle extension (".app")
    if (p)
        *p = '\0';
    strlcat(pathToSelf, "/Contents/MacOS/", sizeof(pathToSelf));
    strlcat(pathToSelf, appName, sizeof(pathToSelf));

    // TODO: Upgrade this to use SMJobBless
    // I have been trying to get the SMJobBless sample application to work
    // but have not yet succeeded.
    // Although SMJobBless is deprecated, the currently recomended SMAppService
    // API is available only since MacOS 13.0, and we want to suppport older
    // versions of MacOS. Also, it is not clear from the documentation how
    // SMAppService allows you to run a helper tool as root after presenting
    // an authorization dialog to the user.
    //
    // The header file deprecation comment for AuthorizationExecuteWithPrivileges() says:
    // Use a launchd-launched helper tool and/or the Service Mangement framework
        myArgv[0] = (char*)"--privileged";
        myArgv[1] = NULL;
        AuthorizationItem authItem		= { kAuthorizationRightExecute, 0, NULL, 0 };
        AuthorizationRights authRights	= { 1, &authItem };
        AuthorizationFlags flags		=	kAuthorizationFlagInteractionAllowed	|
                                            kAuthorizationFlagExtendRights;

        AuthorizationRef authRef = NULL;

        /* Obtain the right to install privileged helper tools (kSMRightBlessPrivilegedHelper). */
        err = AuthorizationCreate(&authRights, kAuthorizationEmptyEnvironment, flags, &authRef);
        if (err == errAuthorizationSuccess) {
            err = AuthorizationExecuteWithPrivileges(authRef, pathToSelf, kAuthorizationFlagDefaults, myArgv, NULL);
        }
        if (err) return err;

    return 0;
}

static Boolean ShowMessage(Boolean allowCancel, Boolean continueButton, Boolean yesNoButtons, const char *format, ...) {
    va_list                 args;
    char                    s[1024];
    CFOptionFlags           responseFlags;
    CFURLRef                myIconURLRef = NULL;
    Boolean                 result;

    va_start(args, format);
    vsnprintf(s, sizeof(s), format, args);
    va_end(args);

    CFStringRef myString = CFStringCreateWithCString(NULL, s, kCFStringEncodingUTF8);
    CFStringRef theTitle = CFStringCreateWithCString(NULL, "Remove Podman", kCFStringEncodingUTF8);
    CFStringRef cancelString = CFStringCreateWithCString(NULL, (char*)("Cancel"), kCFStringEncodingUTF8);
    CFStringRef continueString = CFStringCreateWithCString(NULL, (char*)("Continue..."), kCFStringEncodingUTF8);
    CFStringRef yesString = CFStringCreateWithCString(NULL, (char*)("Yes"), kCFStringEncodingUTF8);
    CFStringRef noString = CFStringCreateWithCString(NULL, (char*)("No"), kCFStringEncodingUTF8);

    // Set default button to Continue, OK or No
    // Set alternate button to Cancel, Yes, or hidden
    SInt32 retval = CFUserNotificationDisplayAlert(0.0, kCFUserNotificationPlainAlertLevel,
                myIconURLRef, NULL, NULL, theTitle, myString,
                continueButton ? continueString : (yesNoButtons ? noString : NULL),
                (allowCancel || yesNoButtons) ? (yesNoButtons ? yesString : cancelString) : NULL,
                NULL, &responseFlags);

    if (myIconURLRef) CFRelease(myIconURLRef);
    if (myString) CFRelease(myString);
    if (theTitle) CFRelease(theTitle);
    if (cancelString) CFRelease(cancelString);
    if (continueString) CFRelease(continueString);
    if (yesString) CFRelease(yesString);
    if (noString) CFRelease(noString);

    if (retval) return false;

    result = (responseFlags == kCFUserNotificationDefaultResponse);
    // Return TRUE if user clicked Continue, Yes or OK, FALSE if user clicked Cancel or No
    // Note: if yesNoButtons is true, we made default button "No" and alternate button "Yes"
    return (yesNoButtons ? !result : result);
}

