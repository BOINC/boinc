/*
 *  SetVersion.c
 *  boinc
 *
 *  Created by Charlie Fenton on 3/29/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */
 
#include <stdio.h>
#include "version.h"

int main(int argc, char** argv) {
    int retval = 0;
    FILE *f;

//#include <unistd.h>
//    char myPath[1024];
//    getcwd(myPath, sizeof(myPath));
//    printf("%s\n", myPath);       // For debugging
    
    f = fopen("./English.lproj/InfoPlist.strings", "w");
    if (f)
    {
        fprintf(f, "/* Localized versions of Info.plist keys */\n\n");
        fprintf(f, "CFBundleName = \"BOINC\";\n");
        fprintf(f, "CFBundleShortVersionString = \"BOINC version %s\";\n", BOINC_VERSION_STRING);
        fprintf(f, "CFBundleGetInfoString = \"BOINC version %s, Copyright 2005 University of California.\";\n", BOINC_VERSION_STRING);
        retval = fclose(f);
    }
    else
        retval = -1;
        
    return retval;
}


#if 0
/* Localized versions of Info.plist keys */

CFBundleName = "BOINC";
CFBundleShortVersionString = "BOINC version 1.03";
CFBundleGetInfoString = "BOINC version 1.03, Copyright 2005 University of California.";
#endif