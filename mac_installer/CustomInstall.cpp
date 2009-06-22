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

/* CustomInstall.cpp */

/* Customizable installer to allow use of features described at 
      http://boinc.berkeley.edu/client_startup.php
Directions for creating a customized installer for the Macintosh:
 [1] Create a directory, with a name such as SETI@home_Mac_Installer
 [2] Place this CustomInstall application inside that directory.  You 
     may rename this application if you wish.
 [3] Create a new  directory named "boinc_startup_files" inside the  
     directory created in step 1.
 [4] Place the custom .xml files in the "boinc_startup_files" directory.
 [5] Zip the directory created in step 1 (with all its contents).
*/

#define CREATE_LOG 1    /* for debugging */

#include <Carbon/Carbon.h>

#include <curl/curl.h>
#include <stdlib.h>
#include "filesys.h"
#include "error_numbers.h"

#ifdef  __cplusplus
extern "C" {
#endif

static void Initialize(void);	/* function prototypes */
static OSStatus download_thread(void* param);
static OSErr download_boinc(FILE * out_file);
//static curl_progress_callback show_prog;
static int show_prog(void *clientp, double dltotal, double dlnow,
                                      double ultotal, double ulnow);
static OSErr runProgressDlog();
static pascal Boolean ProgDlgFilterProc(DialogPtr dp, EventRecord *event, short *item);
static void show_message(StringPtr s1);
static OSErr QuitAppleEventHandler(const AppleEvent *appleEvt, AppleEvent* reply, UInt32 refcon);
static void print_to_log_file(const char *format, ...);
static void strip_cr(char *buf);

#ifdef  __cplusplus
}
#endif

/* globals */
Boolean     gQuitFlag = false;
short       gProgressValue = 0, gOldProgressValue = 0;
char        gCurlError[CURL_ERROR_SIZE];
char        * gDL_URL = "http://boinc.berkeley.edu/dl/boinc_5.2.1_macOSX.zip";
char        * gDownLoadFileName = "boinc_installer.zip";
char        * gCustomDirectoryName = "boinc_startup_files";
MPQueueID   gTerminationQueue;      /* This queue will report the completion of our threads. */
MPTaskID    gDownload_thread_id;    /* IDs of the threads we create. */
CURLcode    gResult;

DialogPtr   gProgressDlog;


int main(int argc, char *argv[])
{
    FILE * boinc_installer_zip_file;
    char path[1024], buf[256], *p;
    DIRREF dirp;
    OSStatus err;
    int retval;

    Initialize();

#ifdef __APPLE__
    if (strlen(argv[0]) >= sizeof(path)) {
        show_message((StringPtr)"\pPath to application is too long.");
        return 0;
    }
    strcpy(path, argv[0]);   // Path to this application.
    p = strstr(path, "/Contents/MacOS/");
    *p = '\0';
    p = strrchr(path, '/');
    *(p+1) = '\0';
    chdir(path);  // Directory containing this application
#if 0   // For testing under debugger
    chdir("/Users/charliefenton/Desktop/FTPTEST/");
    getcwd(path, 256);
#endif

#else   // Not __APPLE__
    getcwd(path, 256);  // Directory containing this application
#endif

    // we use multiple threads to be able to run our progress dialogs during 
    //	copy and search operations
    if (!MPLibraryIsLoaded()) {
        printf("MultiProcessing Library not available.\n");
        ExitToShell();
    }
	
    err = MPCreateQueue(&gTerminationQueue);	/* Create the queue which will report the completion of the task. */
    if (err != noErr) {
        printf("Cannot create the termination queue. err = %d\n", (short)err);
        ExitToShell();
    }

    boinc_installer_zip_file = boinc_fopen(gDownLoadFileName, "w");
    if (boinc_installer_zip_file == NULL)
    {
        show_message((StringPtr)"\pFailed to create zip file.");
        return -1;
    }
    
    err = MPCreateTask(download_thread,             /* This is the task function. */
                        boinc_installer_zip_file,   /* This is the parameter to the task function. */
                        (500*1024),                 /* Stack size for thread. */
                        gTerminationQueue,          /* We'll use this to sense task completion. */
                        0,	/* We won't use the first part of the termination message. */
                        0,	/* We won't use the second part of the termination message. */
                        0,	/* Use the normal task options. (Currently this *must* be zero!) */
                        &gDownload_thread_id);	/* Here's where the ID of the new task will go. */

    if (err != noErr) {
        (void) MPDeleteQueue(gTerminationQueue);
        printf("Cannot create the copy thread. err = %d\n", (short)err);
        ExitToShell();
    }

    err = runProgressDlog();
    
    fclose(boinc_installer_zip_file);

    if (gQuitFlag)
    {
        show_message((StringPtr)"\pCancelled by user.");
        return 0;
    }
    
    if (gResult)
    {
        buf[0] = sprintf(buf+1, "Download error %d:\n%s", gResult, curl_easy_strerror(gResult));
        show_message((StringPtr)buf);
        return 0;
    }
    
    sprintf(buf, "unzip -o %s", gDownLoadFileName);
    retval = system(buf);
    
    if (retval)
    {
        show_message((StringPtr)"\pError expanding downloaded zip file.");
        return 0;
    }

    // Copy the custom XML files
#ifdef __APPLE__
    // On the Macintosh, BOINC puts its data at a fixed, predetermined path
    // so we can just copy the custom files there.  On other platforms, this 
    // application should copy the custom files to a temporary, intermediate 
    // location which the standard installer can then find; the standard 
    // installer should then copy the files to the correct directory and 
    // possibly delete the temporary ones.
    retval = system("mkdir -p /Library/Application\\ Support/BOINC\\ Data");
    if (!retval)
    {
        sprintf(buf, "chmod 0644 .%s%s%s*.xml", PATH_SEPARATOR, gCustomDirectoryName, PATH_SEPARATOR);
        retval = system (buf);
    }
    if (!retval)
    {
        sprintf(buf, "cp -f .%s%s%s*.xml /Library/Application\\ Support/BOINC\\ Data/", 
            PATH_SEPARATOR, gCustomDirectoryName, PATH_SEPARATOR);
        retval = system(buf);
    }
#endif
    if (retval)
    {
        show_message((StringPtr)"\pCouldn't copy custom BOINC startup files.");
        retval = 0;     // Should we continue anyway?
    }
    
    // Search this directory for the expanded BOINC installer directory; it
    // should be the only directory other than the custom files directory.
    sprintf(buf, ".%s", PATH_SEPARATOR);
    
    dirp = dir_open(path);
    if (dirp == NULL)
    {
        retval = ERR_OPENDIR;
    } else {
        do {
            retval = dir_scan(buf+2, dirp, sizeof(buf)-2);
            if (!is_dir(buf))
                continue;
            if (strcmp(buf+2, gCustomDirectoryName))
                break;
        } while (retval == BOINC_SUCCESS);
    }
    
    if (retval)
    {
        show_message((StringPtr)"\pCouldn't find downloaded additional BOINC installer software.");
        return 0;
    }

    // Run the installer
    retval = chdir(buf);
    if (!retval)
        retval = system("open ./BOINC.pkg");

    if (retval)
        show_message((StringPtr)"\pError running additional BOINC installer software.");

    return 0;
}


static void Initialize()	/* Initialize some managers */
{
    OSErr	err;
        
    InitCursor();

    err = AEInstallEventHandler( kCoreEventClass, kAEQuitApplication, NewAEEventHandlerUPP((AEEventHandlerProcPtr)QuitAppleEventHandler), 0, false );
    if (err != noErr)
        ExitToShell();
}


////////////////////////////////////////////////////////////////////////
//                                                                    //
//  CAUTION - download_thread is a second MPThread, so all Mac        //
//  API calls must be thread-safe!                                    //
//                                                                    //
////////////////////////////////////////////////////////////////////////
static OSStatus download_thread(void* param)
{
    return download_boinc((FILE *)param);
}


static OSErr download_boinc(FILE * out_file)
{
    CURL *myHandle;
    CURLcode curlErr;
    
    curlErr = curl_global_init(0);

    myHandle = curl_easy_init();
    if (myHandle == NULL)
    {
        show_message((StringPtr)"\pDownload initialization error.");
        return -1;
    }
    
    curlErr = curl_easy_setopt(myHandle, CURLOPT_VERBOSE, 1);
    curlErr = curl_easy_setopt(myHandle, CURLOPT_NOPROGRESS, 0);
    curlErr = curl_easy_setopt(myHandle, CURLOPT_WRITEDATA, out_file);
    curlErr = curl_easy_setopt(myHandle, CURLOPT_PROGRESSFUNCTION, show_prog);
    curlErr = curl_easy_setopt(myHandle, CURLOPT_PROGRESSDATA, &gProgressValue);
    curlErr = curl_easy_setopt(myHandle, CURLOPT_ERRORBUFFER, gCurlError);
    curlErr = curl_easy_setopt(myHandle, CURLOPT_URL, gDL_URL);

    sleep(2);   // Give progress dialog a chance to be drawn

    curlErr = curl_easy_perform(myHandle);

    curl_easy_cleanup(myHandle);
    curl_global_cleanup();
    
    return curlErr;
}



static int show_prog(void *clientp, double dltotal, double dlnow,
                                      double ultotal, double ulnow)
{
    gProgressValue = (short)(dlnow * 100.0 / dltotal);
    printf("Progress = %d%%\n", gProgressValue);

    return gQuitFlag;
}


static OSErr runProgressDlog()
{
    AlertStdAlertParamRec alertParams;
    DialogItemIndex itemHit = 0;
    ModalFilterUPP myFilterProcUPP = NewModalFilterUPP(ProgDlgFilterProc);
    OSStatus err;

    gOldProgressValue = -1;

    alertParams.movable = false;
    alertParams.helpButton = false;
    alertParams.filterProc = myFilterProcUPP;
    alertParams.defaultText = "\pCancel";
    alertParams.cancelText = NULL;
    alertParams.otherText = NULL;
    alertParams.defaultButton = kAlertStdAlertOKButton;
    alertParams.cancelButton = 0;
    alertParams.position = kWindowDefaultPosition;
    
    ParamText("\pDownloading additional BOINC installer software:\n   0% complete", "\p", "\p", "\p");
    
    err = StandardAlert(kAlertNoteAlert, "\p^0", NULL, &alertParams, &itemHit);
    if (itemHit == kAlertStdAlertOKButton)
        gQuitFlag = true; 

    return noErr;
}


static pascal Boolean ProgDlgFilterProc(DialogPtr dp, EventRecord *event, short *item)
{
    char buf[256];
    OSStatus err;
    
    if (gTerminationQueue)
    {
        err = MPWaitOnQueue(gTerminationQueue, 0, 0, (void **)&gResult, kDurationImmediate);
        if (err == noErr)
        {
            *item = kAlertStdAlertCancelButton;
            return true;
        }
    }
    
    if (gProgressValue == gOldProgressValue)
        return false;
    
    gOldProgressValue = gProgressValue;
    
    buf[0] = sprintf(buf+1, "Downloading additional BOINC installer software:\n %3d%% complete", gProgressValue);
    ParamText("\pDownloading additional BOINC installer software:   0% complete", "\p", "\p", "\p");
    ParamText((StringPtr)buf, "\p", "\p", "\p");
    
    DrawDialog(dp);

    return false;
}


/********************************************************************

	ShowMessage

********************************************************************/
static void show_message(StringPtr s1)
{
    DialogItemIndex itemHit;
    OSErr err;
    
    err = StandardAlert (kAlertStopAlert, s1, NULL, NULL, &itemHit);
}


static OSErr QuitAppleEventHandler( const AppleEvent *appleEvt, AppleEvent* reply, UInt32 refcon )
{
    gQuitFlag =  true;
    
    return noErr;
}


// For debugging
static void print_to_log_file(const char *format, ...) {
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
static void strip_cr(char *buf)
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
