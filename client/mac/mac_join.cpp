// The contents of this file are subject to the BOINC Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://boinc.berkeley.edu/license_1.0.txt
// 
// Software distributed under the License is distributed on an "AS IS"
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
// License for the specific language governing rights and limitations
// under the License. 
// 
// The Original Code is the Berkeley Open Infrastructure for Network Computing. 
// 
// The Initial Developer of the Original Code is the SETI@home project.
// Portions created by the SETI@home project are Copyright (C) 2002
// University of California at Berkeley. All Rights Reserved. 
// 
// Contributor(s):
//

#include "mac_join.h"

bool success;

#define kJoinSignature        'join'
#define kProjectURLFieldID    128
#define kAccountKeyFieldID    129

// Create, show and run modally our join window
//
OSStatus CreateJoinDialog( char *master_url, char *account_key )
{
    IBNibRef         nibRef;
    EventTypeSpec     dialogSpec = {kEventClassCommand, kEventCommandProcess };
    WindowRef         dialogWindow;
    EventHandlerUPP    dialogUPP;
    OSStatus        err = noErr;
    Size        realSize;
    ControlID        projectURLID = { kJoinSignature, kProjectURLFieldID };
    ControlID        accountKeyID = { kJoinSignature, kAccountKeyFieldID };
    ControlHandle    zeControl;
    CFStringRef        text;
    
    success = false;
    
    // Find the dialog nib
    err = CreateNibReference(CFSTR("JoinDialog"), &nibRef);
    require_noerr( err, CantFindDialogNib );

    // Load the window inside it
    err = CreateWindowFromNib(nibRef, CFSTR("Join Dialog"), &dialogWindow);
    require_noerr( err, CantCreateDialogWindow );

    // We don't need the nib reference anymore.
    DisposeNibReference(nibRef);

    // Install our event handler
    dialogUPP =  NewEventHandlerUPP (JoinDialogEventHandler);
    err = InstallWindowEventHandler (dialogWindow, dialogUPP, 1, &dialogSpec, (void *) dialogWindow, NULL);
    require_noerr( err, CantInstallDialogHandler );

    // Show the window
    ShowWindow( dialogWindow );
    
    // Run modally
    RunAppModalLoopForWindow(dialogWindow);

    GetControlByID( dialogWindow, &projectURLID, &zeControl );
    GetControlData( zeControl, 0, kControlEditTextCFStringTag, sizeof(CFStringRef), &text, &realSize);
    CFStringGetCString( text, master_url, 256, CFStringGetSystemEncoding() );

    GetControlByID( dialogWindow, &accountKeyID, &zeControl );
    GetControlData( zeControl, 0, kControlEditTextCFStringTag, sizeof(CFStringRef), &text, &realSize);
    CFStringGetCString( text, account_key, 256, CFStringGetSystemEncoding() );
    
    HideWindow(dialogWindow);
    DisposeWindow(dialogWindow);
    DisposeEventHandlerUPP(dialogUPP);

CantFindDialogNib:
CantCreateDialogWindow:
CantInstallDialogHandler:

    return success;
}

// Dialog event handler
//
pascal OSStatus JoinDialogEventHandler (EventHandlerCallRef myHandler, EventRef event, void *userData) {
    OSStatus         result = eventNotHandledErr;
    HICommand        command;
    bool        stopModalLoop = FALSE;

    // Get the HI Command
    GetEventParameter (event, kEventParamDirectObject, typeHICommand, NULL,
                       sizeof (HICommand), NULL, &command);
    // Look for our Yes Join and No Join commands
    switch (command.commandID) {
        case kHICommandOK:        // 'ok  '
            success = true;
        case kHICommandCancel:        // 'not!'
            stopModalLoop = TRUE;
            result = noErr;
            break;
    }

    // Stop the modal loop.
    if (stopModalLoop) {
        QuitAppModalLoopForWindow((WindowRef)userData);
    }

    //Return how we handled the event.
    return result;
}

const char *BOINC_RCSID_f85123c042 = "$Id$";
