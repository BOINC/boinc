// Berkeley Open Infrastructure for Network Computing
// http://boinc.berkeley.edu
// Copyright (C) 2005 University of California
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
