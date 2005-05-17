/*
 *  SetVersion.c
 *  boinc
 *
 *  Created by Charlie Fenton on 3/29/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

// Set STAND_ALONE TRUE if testing as a separate applicaiton
#define STAND_ALONE 0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "version.h"

#if STAND_ALONE
#include <unistd.h>     // for getcwd if debugging
#endif

int IsFileCurrent(char* filePath);
int FixInfoPlistFile(char* myPath);
int FixInfoPlist_Strings(void);

int main(int argc, char** argv) {
    int retval = 0, err;

#if STAND_ALONE
    char myPath[1024];
    getcwd(myPath, sizeof(myPath));
    printf("%s\n", myPath);       // For debugging
    err = chdir("../");
    getcwd(myPath, sizeof(myPath));
    printf("%s\n", myPath);       // For debugging
#endif

    err = FixInfoPlist_Strings();
    if (err) retval = err;
    FixInfoPlistFile("./Info.plist");
    if (err) retval = err;
    FixInfoPlistFile("./Installer-Info.plist");
    if (err) retval = err;
    FixInfoPlistFile("./ScreenSaver-Info.plist");
    if (err) retval = err;
    FixInfoPlistFile("./SystemMenu-Info.plist");
    if (err) retval = err;
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


int FixInfoPlist_Strings() {
    int retval = 0;
    FILE *f;
    char *myPath = "./English.lproj/InfoPlist.strings";
    
    if (IsFileCurrent(myPath))
        return 0;

    f = fopen(myPath, "w");
    if (f)
    {
        fprintf(f, "/* Localized versions of Info.plist keys */\n\n");
        fprintf(f, "CFBundleName = \"BOINC\";\n");
        fprintf(f, "CFBundleShortVersionString = \"BOINC version %s\";\n", BOINC_VERSION_STRING);
        fprintf(f, "CFBundleGetInfoString = \"BOINC version %s, Copyright 2005 University of California.\";\n", BOINC_VERSION_STRING);
        retval = fclose(f);
    }
    else {
        puts("Error updating version number in file InfoPlist.strings\n");
        retval = -1;
    }
        
    return retval;
}


int FixInfoPlistFile(char* myPath) {
    int retval = 0;
    FILE *fin = NULL, *fout = NULL;
    char *c, a, buf[1024];
    
    if (IsFileCurrent(myPath))
        return 0;

    fin = fopen(myPath, "r");
    if (fin == NULL)
        goto bail;

    fout = fopen("temp", "w");
    if (fout == NULL) {
        fclose(fin);
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
    fclose(fout);
    
    sprintf(buf, "mv -f temp %s", myPath);
    retval = system(buf);
    
    return retval;

bail:
    if (fin)
        fclose(fin);
    if (fout)
        fclose(fout);

    printf("Error updating version number in file %s\n", myPath);
    return -1;
}
