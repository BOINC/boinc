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


#define boinc_master_user_name "boinc_master"
#define boinc_master_group_name "boinc_master"
#define boinc_project_user_name "boinc_project"
#define boinc_project_group_name "boinc_project"

void Initialize(void);	/* function prototypes */
Boolean IsUserMemberOfGroup(const char *userName, const char *groupName);
OSStatus GetFinalAction(CFStringRef *restartValue);
OSErr FindProcess (OSType typeToFind, OSType creatorToFind, ProcessSerialNumberPtr processSN);
static OSErr QuitAppleEventHandler(const AppleEvent *appleEvt, AppleEvent* reply, UInt32 refcon);
void print_to_log_file(const char *format, ...);
void strip_cr(char *buf);

Boolean			gQuitFlag = false;	/* global */

CFStringRef     valueRestartRequired = CFSTR("RequiredRestart");
CFStringRef     valueLogoutRequired = CFSTR("RequiredLogout");
CFStringRef     valueNoRestart = CFSTR("NoRestart");


int main(int argc, char *argv[])
{
    char                    pkgPath[MAXPATHLEN], infoPlistPath[MAXPATHLEN], MetaPkgPath[MAXPATHLEN];
    char                    brand[64], s[256];
    char                    *p;
    ProcessSerialNumber     ourPSN, installerPSN;
    FSRef                   ourFSRef;
    long                    response;
    short                   itemHit;
    pid_t                   installerPID = 0;
    FSRef                   infoPlistFileRef;
    Boolean                 isDirectory, result;
    CFURLRef                xmlURL = NULL;
    CFDataRef               xmlDataIn = NULL;
    CFDataRef               xmlDataOut = NULL;
    CFPropertyListRef       propertyListRef = NULL;
    CFStringRef             restartKey = CFSTR("IFPkgFlagRestartAction");
    CFStringRef             currentValue = NULL, desiredValue = NULL;
    CFStringRef             errorString = NULL;
    OSStatus                err = noErr;
    struct stat             stat_buf;

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
    strlcat(pkgPath, ".pkg", sizeof(pkgPath));
    err = Gestalt(gestaltSystemVersion, &response);
    if (err != noErr)
        return err;

    if (response < 0x1040) {
        ::SetFrontProcess(&ourPSN);
        p = strrchr(brand, ' ');         // Strip off last space character and everything following
        if (p)
            *p = '\0'; 
        s[0] = sprintf(s+1, "Sorry, this version of %s requires system 10.4 or higher.", brand);
        StandardAlert (kAlertStopAlert, (StringPtr)s, NULL, NULL, &itemHit);

        err = FindProcess ('APPL', 'xins', &installerPSN);
        err = GetProcessPID(&installerPSN , &installerPID);

        if (err == noErr)
            err = kill(installerPID, SIGKILL);

	ExitToShell();
    }

    // Remove previous installer package receipt so we can run installer again
    // "rm -rf /Library/Receipts/GridRepublic.pkg"
    sprintf(s, "rm -rf \"/Library/Receipts/%s.pkg\"", brand);
    system (s);

    err = Gestalt(gestaltSystemVersion, &response);
    if (err != noErr)
        return err;
    
    err = GetFinalAction(&desiredValue);
    
    strlcpy(infoPlistPath, pkgPath, sizeof(infoPlistPath));
    strlcat(infoPlistPath, "/Contents/Info.plist", sizeof(infoPlistPath));

    err = FSPathMakeRef((UInt8*)infoPlistPath, &infoPlistFileRef, &isDirectory);
    if (err)
        return err;
        
    xmlURL = CFURLCreateFromFSRef(NULL, &infoPlistFileRef);
    if (xmlURL == NULL)
        return -1;

    // Read XML Data from file
    result = CFURLCreateDataAndPropertiesFromResource(NULL, xmlURL, &xmlDataIn, NULL, NULL, &err);
    if (err == noErr)
        if (!result)
            err = coreFoundationUnknownErr;
	
    if (err == noErr) { // Convert XML Data to internal CFPropertyListRef / CFDictionaryRef format
        propertyListRef = CFPropertyListCreateFromXMLData(NULL, xmlDataIn, kCFPropertyListMutableContainersAndLeaves, &errorString);
        if (propertyListRef == NULL)
            err = coreFoundationUnknownErr;
    }
    
    if (err == noErr) { // Get current value for our key
        currentValue = (CFStringRef)CFDictionaryGetValue((CFDictionaryRef)propertyListRef, restartKey);
        if (currentValue == NULL)
            err = coreFoundationUnknownErr;
    }    
    
    if (err == noErr) {
        if (CFStringCompare(currentValue, desiredValue, 0) != kCFCompareEqualTo) {  // If current value != desired value
            // Replace value for key with desired value
            CFDictionaryReplaceValue((CFMutableDictionaryRef)propertyListRef, restartKey, desiredValue);

            // Convert internal CFPropertyListRef / CFDictionaryRef format to XML data
            xmlDataOut = CFPropertyListCreateXMLData(NULL, propertyListRef);
            if (xmlDataOut == NULL)
                err = coreFoundationUnknownErr;

            if (err == noErr) { // Write revised XML Data back to the file
                result = CFURLWriteDataAndPropertiesToResource (xmlURL, xmlDataOut, NULL, &err);
                if (err == noErr)
                    if (!result)
                        err = coreFoundationUnknownErr;
            }
        }
    } 
   
    if (xmlURL)
        CFRelease(xmlURL);
    if (xmlDataIn)
        CFRelease(xmlDataIn);
    if (xmlDataOut)
        CFRelease(xmlDataOut);
    if (propertyListRef)
        CFRelease(propertyListRef);

    if (err == noErr) {
        if ((response < 0x1050) || stat(MetaPkgPath, &stat_buf)) {  // stat() returns zero on success
            sprintf(infoPlistPath, "open \"%s\" &", pkgPath);
        } else {
            sprintf(infoPlistPath, "open \"%s\" &", MetaPkgPath);
        }
        system(infoPlistPath);
    }

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


OSStatus GetFinalAction(CFStringRef *restartValue)
{
    passwd          *pw = NULL;
    group           *grp = NULL;
    gid_t           boinc_master_gid = 0, boinc_project_gid = 0;
    uid_t           boinc_master_uid = 0, boinc_project_uid = 0;
    OSStatus        err = noErr;
    long            response;
    char            loginName[256];
    
    *restartValue = valueRestartRequired;

    grp = getgrnam(boinc_master_group_name);
    if (grp == NULL)
        return noErr;       // Group boinc_master does not exist

    boinc_master_gid = grp->gr_gid;
        
    grp = getgrnam(boinc_project_group_name);
    if (grp == NULL)
        return noErr;       // Group boinc_project does not exist

    boinc_project_gid = grp->gr_gid;

    pw = getpwnam(boinc_master_user_name);
    if (pw == NULL)
        return noErr;       // User boinc_master does not exist

    boinc_master_uid = pw->pw_uid;

    if (pw->pw_gid != boinc_master_gid)
        return noErr;       // User boinc_master does not have group boinc_master as its primary group
    
    pw = getpwnam(boinc_project_user_name);
    if (pw == NULL)
        return noErr;       // User boinc_project does not exist

    boinc_project_uid = pw->pw_uid;
        
    if (pw->pw_gid != boinc_project_gid)
        return noErr;       // User boinc_project does not have group boinc_project as its primary group

    err = Gestalt(gestaltSystemVersion, &response);
    if ((err == noErr) && (response >= 0x1050)) {
        if (boinc_master_gid < 501)
            return noErr;       // We will change boinc_master_gid to a value > 501
        if (boinc_project_gid < 501)
            return noErr;       // We will change boinc_project_gid to a value > 501
        if (boinc_master_uid < 501)
            return noErr;       // We will change boinc_master_uid to a value > 501
        if (boinc_project_uid < 501)
            return noErr;       // We will change boinc_project_uid to a value > 501
}
    
    #ifdef SANDBOX
    strncpy(loginName, getenv("USER"), sizeof(loginName)-1);
    if (loginName[0]) {
        if (IsUserMemberOfGroup(loginName, boinc_master_group_name)) {
            *restartValue = valueNoRestart;
            return noErr;   // Logged in user is already a member of group boinc_master
        }
    }
#endif  // SANDBOX

    return noErr;
}


void Initialize()	/* Initialize some managers */
{
    OSErr	err;
        
//    InitCursor();

    err = AEInstallEventHandler( kCoreEventClass, kAEQuitApplication, NewAEEventHandlerUPP((AEEventHandlerProcPtr)QuitAppleEventHandler), 0, false );
    if (err != noErr)
        ExitToShell();
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
    strcpy(buf, getenv("HOME"));
    strcat(buf, "/Documents/test_log.txt");
    f = fopen(buf, "a");
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
