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

#include <math.h>
#include <stdio.h>
#include <string.h>

// project includes ---------------------------------------------------------

#include "mac_main.h"
#include "mac_join.h"

// statics/globals (internal only) ------------------------------------------

static const EventTypeSpec  appEventList[] =
{
{kEventClassCommand, kEventCommandProcess},
    	{kEventClassMouse, kEventMouseDown},
{kEventClassMouse, kEventMouseUp},
{kEventClassMouse, kEventMouseMoved},
{kEventClassMouse, kEventMouseDragged},
{kEventClassMouse, kEventMouseWheelMoved}
};

WindowRef               boincWindow;
ControlRef              boincStatusControl;
EventLoopTimerRef 	boincTimer;
EventLoopTimerUPP	boincTimerUPP;
EventHandlerUPP 	appCommandProcessor;
WindowPtr               boincAboutWindow;

bool user_requested_exit = false;

#define kStatusControl          'cntl'
#define kStatusControlID        128

enum
{
    kBOINCCommandJoin = 'join',
    kBOINCCommandSuspend = 'susp'
};

// --------------------------------------------------------------------------

void InitToolbox(void)
{
    OSStatus err;
    Handle boincMenuBar;
    ControlID ctrlID;
    IBNibRef boincNibRef;

    InitCursor();
    
    // Search for the "boinc" .nib file
    err = CreateNibReference(CFSTR("boinc"), &boincNibRef);
    if ( err != noErr ) {
        fprintf(stderr, "Can't load boinc.nib. Err: %d\n", (int)err);
        ExitToShell();
    }        

    // Init Menus
    err = CreateMenuBarFromNib(boincNibRef, CFSTR("MainMenu"), &boincMenuBar);
    if ( err != noErr ) {
        fprintf(stderr, "Can't load MenuBar. Err: %d\n", (int)err);
        ExitToShell();
    }

    err = CreateWindowFromNib(boincNibRef, CFSTR("Client Window"), &boincWindow);
    if (err != noErr) {
        fprintf(stderr, "Can't load Window. Err: %d\n", (int)err);
        ExitToShell();
    }
    
    ctrlID.id = kStatusControl;
    ctrlID.signature = kStatusControlID;
    GetControlByID(boincWindow, &ctrlID, &boincStatusControl);

    // Enable the preferences item
    EnableMenuCommand(NULL, kHICommandPreferences);
    
    // Application-level event handler installer
    appCommandProcessor = NewEventHandlerUPP(MainAppEventHandler);
    err = InstallApplicationEventHandler(appCommandProcessor, GetEventTypeCount(appEventList),
                                            appEventList, 0, NULL);
    // BOINC Timed event handler installer
    boincTimerUPP = NewEventLoopTimerUPP(BOINCPollLoopProcessor);
    err = InstallEventLoopTimer(GetCurrentEventLoop(), 0,
                                kEventDurationMillisecond*1000,		// Every 1 second
                                boincTimerUPP, NULL, &boincTimer);

    SetMenuBar(boincMenuBar);
    ShowWindow(boincWindow);
    
    DisposeNibReference(boincNibRef);
}

//////////////////////////////////////////////////////////////////////////////////
//  BOINCPollLoopProcessor							//
//////////////////////////////////////////////////////////////////////////////////
pascal void BOINCPollLoopProcessor(EventLoopTimerRef inTimer, void* timeData)
{
#pragma unused(inTimer, timeData)
    fprintf( stderr, "Fired timer.\n" );
    // Call do_something here

}

//////////////////////////////////////////////////////////////////////////////////
//  MainAppEventHandler								//
//////////////////////////////////////////////////////////////////////////////////
pascal OSStatus MainAppEventHandler(EventHandlerCallRef appHandler, EventRef theEvent, void* appData)
{
#pragma unused (appHandler, appData)

    HICommand	aCommand;
    OSStatus	result;
    Point	mDelta;

    switch(GetEventClass(theEvent))
    {
        case kEventClassMouse:				// 'mous'
            GetEventParameter(theEvent,			// the event itself
                              kEventParamMouseDelta, 	// symbolic parameter name
                              typeQDPoint, 		// expected type
                              NULL, 			// actual type (NULL is valid)
                              sizeof(mDelta), 		// buffer size
                              NULL, 			// actual buffer size (Can be NULL)
                              &mDelta);			// variable to hold data
            switch(GetEventKind(theEvent))
            {
                case kEventMouseDown:
                    break;
                case kEventMouseUp:
                    break;
                case kEventMouseMoved:
                    break;
                case kEventMouseDragged:
                    break;
                case kEventMouseWheelMoved:
                    break;
                default:
                    result = eventNotHandledErr;
                    break;
            }
                break;
        case kEventClassCommand:
            result = GetEventParameter(theEvent, kEventParamDirectObject,
                                       typeHICommand, NULL, sizeof(HICommand),
                                       NULL, &aCommand);
            switch (aCommand.commandID)
            {
                case kHICommandOK:		// 'ok  '
                case kHICommandCancel:		// 'not!'
                    result = eventNotHandledErr;
                    break;
                case kHICommandQuit:
                    QuitApplicationEventLoop();
                    result = noErr;
                    break;
                case kHICommandUndo:		// 'undo'
                case kHICommandRedo:		// 'redo'
                case kHICommandCut:		// 'cut '
                case kHICommandCopy:		// 'copy'
                case kHICommandPaste:		// 'past'
                case kHICommandClear:		// 'clea'
                case kHICommandSelectAll:	// 'sall'
                case kHICommandHide:		// 'hide'
                case kHICommandZoomWindow:	// 'zoom'
                case kHICommandMinimizeWindow:	// 'mini'
                case kHICommandArrangeInFront:	// 'frnt'
                    break;
                case kHICommandPreferences:	// 'pref'
                    // Open prefs dialog
                    result = noErr;
                    break;
                case kBOINCCommandJoin:	// 'join'
                    // Open join dialog
                    CreateJoinDialog();
                    result = noErr;
                    break;
                case kBOINCCommandSuspend:	// 'susp'
                    // Suspend processing
                    result = noErr;
                    break;
                case kHICommandAbout:		// 'abou'
                    // Put in check to see if window is already open
                    /*err = CreateWindowFromNib(boincNibRef, CFSTR("About Box"), &boincAboutWindow);
                    if (err != noErr) {
                        fprintf(stderr, "Can't load Window. Err: %d\n", (int)err);
                        ExitToShell();
                    }
                    ShowWindow(boincAboutWindow);*/
                    result = noErr;
                    break;
                default:
                    result = eventNotHandledErr;
                    break;
            }
                break;
        default:
            result = eventNotHandledErr;
            break;
    }
    return result;
}

// --------------------------------------------------------------------------

int mac_setup (void)
{
    InitToolbox ();
    RunApplicationEventLoop();
    return true;
}

// --------------------------------------------------------------------------

void mac_cleanup (void)
{
    RemoveEventLoopTimer(boincTimer);
    DisposeEventLoopTimerUPP(boincTimerUPP);
    DisposeEventHandlerUPP(appCommandProcessor);
}
