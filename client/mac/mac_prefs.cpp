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

#include "mac_prefs.h"

// Create, show and run our prefs dialog
//
OSStatus CreatePrefsDialog()
{
    IBNibRef         nibRef;
    EventTypeSpec     dialogSpec = {kEventClassCommand, kEventCommandProcess };
    WindowRef         prefsDialog;
    EventHandlerUPP    prefsDialogUPP;
    OSStatus        err = noErr;

    // Find the dialog nib
    err = CreateNibReference(CFSTR("PrefsDialog"), &nibRef);
    require_noerr( err, CantFindDialogNib );

    // Load the window inside it
    err = CreateWindowFromNib(nibRef, CFSTR("Prefs Dialog"), &prefsDialog);
    require_noerr( err, CantCreateDialogWindow );

    // We don't need the nib reference anymore.
    DisposeNibReference(nibRef);

    // Install our event handler
    prefsDialogUPP =  NewEventHandlerUPP (PrefsDialogEventHandler);
    err = InstallWindowEventHandler (prefsDialog, prefsDialogUPP, 1, &dialogSpec, (void *) prefsDialog, NULL);
    require_noerr( err, CantInstallDialogHandler );

    // Show the window
    ShowWindow( prefsDialog );

    // Run modally
    RunAppModalLoopForWindow(prefsDialog);
    
    HideWindow(prefsDialog);
    DisposeWindow(prefsDialog);
    DisposeEventHandlerUPP(prefsDialogUPP);

CantFindDialogNib:
CantCreateDialogWindow:
CantInstallDialogHandler:

        return err;
}


// Dialog event handler
//
pascal OSStatus PrefsDialogEventHandler (EventHandlerCallRef myHandler, EventRef event, void *userData) {
    OSStatus         result = eventNotHandledErr;
    HICommand        command;
    bool        stopModalLoop = FALSE;
    
    // Get the HI Command
    GetEventParameter (event, kEventParamDirectObject, typeHICommand, NULL,
                    sizeof (HICommand), NULL, &command);
    // Look for OK and Cancel commands
    switch (command.commandID) {
        case kHICommandOK:        // 'ok  '
            //HandleResponse(TRUE);
            stopModalLoop = TRUE;
            result = noErr;
            break;
        case kHICommandCancel:        // 'not!'
            //HandleResponse(FALSE);
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

