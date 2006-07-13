// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2006 University of California
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation;
// either version 2.1 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// To view the GNU Lesser General Public License visit
// http://www.gnu.org/copyleft/lesser.html
// or write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

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


#define boinc_master_user_name "boinc_master"
#define boinc_master_group_name "boinc_master"
#define boinc_project_user_name "boinc_project"
#define boinc_project_group_name "boinc_project"

void Initialize(void);	/* function prototypes */
OSStatus IsLogoutNeeded(Boolean *result);
OSErr UpdateAllVisibleUsers(long brandID);
long GetBrandID(void);
static OSErr QuitAppleEventHandler(const AppleEvent *appleEvt, AppleEvent* reply, UInt32 refcon);
void print_to_log_file(const char *format, ...);
void strip_cr(char *buf);

Boolean			gQuitFlag = false;	/* global */

int main(int argc, char *argv[])
{
    char                    pkgPath[MAXPATHLEN], infoPlistPath[MAXPATHLEN];
    char                    *p;
    ProcessSerialNumber     ourPSN;
    FSRef                   ourFSRef;
    FSRef                   infoPlistFileRef;
    Boolean                 isDirectory, result;
    CFURLRef                xmlURL = NULL;
    CFDataRef               xmlDataIn = NULL;
    CFDataRef               xmlDataOut = NULL;
    CFPropertyListRef       propertyListRef = NULL;
    CFStringRef             restartKey = CFSTR("IFPkgFlagRestartAction");
    CFStringRef             currentValue = NULL, desiredValue = NULL;
    CFStringRef             valueLogoutRequired = CFSTR("RequiredLogout");
    CFStringRef             valueNoRestart = CFSTR("NoRestart");
    CFStringRef             errorString = NULL;
    Boolean                 needLogout;
    OSStatus                err = noErr;
    
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
    strlcat(pkgPath, p+1, sizeof(pkgPath));
    p = strrchr(pkgPath, ' ');         // Strip off space characterand everything following )
    if (p)
        *p = '\0'; 

    strlcat(pkgPath, ".pkg", sizeof(pkgPath));
    
    err = IsLogoutNeeded(&needLogout);
    desiredValue = needLogout ? valueLogoutRequired : valueNoRestart;
    
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
        sprintf(infoPlistPath, "open \"%s\" &", pkgPath);
        system(infoPlistPath);
    }

    return err;
}


OSStatus IsLogoutNeeded(Boolean *result)
{
    long            response;
    OSStatus        err = noErr;
    passwd          *pw = NULL;
    group           *grp = NULL;
    gid_t           boinc_master_gid = 0, boinc_project_gid = 0;
    uid_t           boinc_master_uid = 0, boinc_project_uid = 0;
    short           i, j;
    char            *p;
    DIR             *dirp;
    dirent          *dp;
    
    *result = true;

    err = Gestalt(gestaltSystemVersion, &response);
    if (err != noErr)
        return err;
    
    if (response >= 0x1040) {   // Logout is never needed on OS 10.4 and later
        *result = false;
        return noErr;
    }
    
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
        
    for (i=0; ; i++) {              // Step through all users in group boinc_project
        p = grp->gr_mem[i];
        if (p == NULL)
            return noErr;           // User boinc_master is not a member of group boinc_project
        if (strcmp(p, boinc_master_user_name) == 0)
            break;
    }

    // Step through all visible users.  If user is a member of group admin, verify 
    // that user is also a member of both groups boinc_master and boinc_project.
    // NOTE: getgrnam and getgrgid use one static memory area to return their results, 
    //  so each call to getgrnam or getgrgid overwrites the data from any previous calls.
    dirp = opendir("/Users");
    if (dirp == NULL)           // Should never happen
        return noErr;
    
    while (true) {
        dp = readdir(dirp);
        if (dp == NULL)
            break;                  // End of list
            
        if (dp->d_name[0] == '.')
            continue;               // Ignore names beginning with '.'
    
        pw = getpwnam(dp->d_name);
        if (pw == NULL)             // "Deleted Users", "Shared", etc.
            continue;

        for (j=0; ; j++) {                              // Step through all users in group admin
            grp = getgrnam("admin");                    // See NOTE above
            if (grp == NULL)                            // Should never happen
                return noErr;

           p = grp->gr_mem[j];
            if(p == NULL)
                break;              // User is not a member of group admin
                
            if (strcmp(p, dp->d_name) == 0) {
                // User is a member of group admin, so should also be member of groups boinc_master and boinc_project

                grp = getgrgid(boinc_master_gid);       // See NOTE above
                if (grp == NULL)                        // Should never happen
                    return noErr;

                for (i=0; ; i++) {                      // Step through all users in group boinc_master
                    p = grp->gr_mem[i];
                    if (p == NULL)
                        return noErr;           // User is not a member of group boinc_master
                    if (strcmp(p, dp->d_name) == 0)
                        break;
                }

                grp = getgrgid(boinc_project_gid);      // See NOTE above
                if (grp == NULL)                        // Should never happen
                    return noErr;

                for (i=0; ; i++) {                      // Step through all users in group boinc_project
                    p = grp->gr_mem[i];
                    if (p == NULL)
                        return noErr;           // User is not a member of group boinc_project
                    if (strcmp(p, dp->d_name) == 0)
                        break;
                }

                break;       // Check next user
            }       // End if (strcmp(p, dp->d_name) == 0)
        }           // End for j
    }               // End stepping through /Users directory
    
    closedir(dirp);





    *result = false;

    return noErr;
}


void Initialize()	/* Initialize some managers */
{
    OSErr	err;
        
    InitCursor();

    err = AEInstallEventHandler( kCoreEventClass, kAEQuitApplication, NewAEEventHandlerUPP((AEEventHandlerProcPtr)QuitAppleEventHandler), 0, false );
    if (err != noErr)
        ExitToShell();
}


long GetBrandID()
{
    long iBrandId;

    iBrandId = 0;   // Default value
    
    FILE *f = fopen("Contents/Resources/Branding", "r");
    if (f) {
        fscanf(f, "BrandId=%ld\n", &iBrandId);
        fclose(f);
    }
    
    return iBrandId;
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
const char *BOINC_RCSID_c7abe0490e="$Id$";
