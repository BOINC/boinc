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

const char *BOINC_RCSID_c5d2599d58 = "$Id$";
