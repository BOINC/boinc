// The contents of this file are subject to the Mozilla Public License
// Version 1.0 (the "License"); you may not use this file except in
// compliance with the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/
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

#include "mac_about.h"

// Create, show and run our about box
//
OSStatus CreateAboutWindow()
{
    IBNibRef         nibRef;
    EventTypeSpec     dialogSpec = {kEventClassCommand, kEventCommandProcess };
    WindowRef         aboutWindow;
    EventHandlerUPP    aboutBoxUPP;
    OSStatus        err = noErr;

    // Find the dialog nib
    err = CreateNibReference(CFSTR("AboutBox"), &nibRef);
    require_noerr( err, CantFindDialogNib );

    // Load the window inside it
    err = CreateWindowFromNib(nibRef, CFSTR("About Box"), &aboutWindow);
    require_noerr( err, CantCreateDialogWindow );

    // We don't need the nib reference anymore.
    DisposeNibReference(nibRef);

    // Install our event handler
    /*dialogUPP =  NewEventHandlerUPP (JoinDialogEventHandler);
    err = InstallWindowEventHandler (aboutWindow, aboutBoxUPP, 1, &dialogSpec, (void *) dialogWindow, NULL);
    require_noerr( err, CantInstallDialogHandler );*/

    // Show the window
    ShowWindow( aboutWindow );

    /*HideWindow(aboutWindow);
    DisposeWindow(aboutWindow);
    DisposeEventHandlerUPP(aboutBoxUPP);*/

CantFindDialogNib:
CantCreateDialogWindow:
CantInstallDialogHandler:

        return err;
}


// Dialog event handler
//
/*pascal OSStatus AboutBoxEventHandler (EventHandlerCallRef myHandler, EventRef event, void *userData) {
    OSStatus         result = eventNotHandledErr;
    HICommand        command;
    bool        stopModalLoop = FALSE;

    // Get the HI Command
    GetEventParameter (event, kEventParamDirectObject, typeHICommand, NULL,
                       sizeof (HICommand), NULL, &command);
    // Look for our Yes Join and No Join commands
    switch (command.commandID) {
        case 'ysjn':
            //HandleResponse(TRUE);
            stopModalLoop = TRUE;
            result = noErr;
            break;
        case 'nojn':
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

*/