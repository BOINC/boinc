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


const char *BOINC_RCSID_362d5df8ab = "$Id$";
