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

#include "graphics_api.h"

#include "mac_app_opengl.h"
#include "mac_carbon_gl.h"

#ifdef __APPLE_CC__
    #include <Carbon/Carbon.h>
    #include <OpenGL/gl.h>
#else
    #include <gl.h>
    
    #include <Devices.h>
    #include <Dialogs.h>
    #include <DriverServices.h>
    #include <Events.h>
    #include <Gestalt.h>
    #include <LowMem.h>
    #include <PictUtils.h>
    #include <TextEdit.h>
    #include <ToolUtils.h>
    #include <QDOffscreen.h>
    #include <Windows.h>
#endif

#include <math.h>
#include <stdio.h>
#include <string.h>

// project includes ---------------------------------------------------------

#include "mac_main.h"

static void DisposeGLWindow (WindowPtr pWindow); // Dispose a single window and it's GL context

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

WindowRef               appGLWindow;
EventLoopTimerRef 	boincTimer;
EventLoopTimerUPP	boincTimerUPP;
EventHandlerUPP 	appCommandProcessor;
WindowPtr               boincAboutWindow;
AGLContext		boincAGLContext;
GLuint			monacoFontList;
structGLWindowInfo	glInfo;

bool user_requested_exit = false;

// --------------------------------------------------------------------------

int InitGLWindow(int xsize, int ysize, int depth)
{
    OSStatus err;
    Rect winRect;
    TimerUPP boincYieldUPP;
    EventLoopTimerRef boincYieldTimer;
    short i,fNum;
    long response;
    MenuHandle menu;
    
    InitCursor();
    
    // add quit if not under Mac OS X
    err = Gestalt (gestaltMenuMgrAttr, &response);
    if ((err == noErr) && !(response & gestaltMenuMgrAquaLayoutMask)) {
        menu = NewMenu (128, "\pFile");			// new menu
        InsertMenu (menu, 0);				// add menu to end
    
        AppendMenu (menu, "\pQuit/Q"); 			// add quit
    }

    SetRect( &winRect, 100, 100, 100+xsize, 100+ysize );
    
    err = CreateNewWindow ( kDocumentWindowClass, kWindowStandardDocumentAttributes, &winRect, &appGLWindow );
    if (err != noErr) return -1;
    
    // Application-level event handler installer
    appCommandProcessor = NewEventHandlerUPP(MainAppEventHandler);
    err = InstallApplicationEventHandler(appCommandProcessor, GetEventTypeCount(appEventList),
                                            appEventList, 0, NULL);

    // Install preemption
    boincYieldUPP = NewEventLoopTimerUPP(YieldProcessor);
    err = InstallEventLoopTimer(GetMainEventLoop(), 0,
                                kEventDurationMillisecond*10,		// Every 10 ms
                                boincYieldUPP, NULL, &boincYieldTimer);

    // Install graphics
    boincYieldUPP = NewEventLoopTimerUPP(GraphicsLoopProcessor);
    err = InstallEventLoopTimer(GetMainEventLoop(), 0,
                                kEventDurationMillisecond*10,		// Every 10 ms
                                boincYieldUPP, NULL, &boincYieldTimer);
    
    // TODO: add an event handler for the window
    ChangeWindowAttributes( appGLWindow, kWindowStandardHandlerAttribute, 0 );
    SetWTitle (appGLWindow, "\pWindow");
    ShowWindow(appGLWindow);
    SetPortWindowPort (appGLWindow);
    
    glInfo.fAcceleratedMust = false; 	// must renderer be accelerated?
    glInfo.VRAM = 0 * 1048576;		// minimum VRAM (if not zero this is always a required minimum)
    glInfo.textureRAM = 0 * 1048576;	// minimum texture RAM (if not zero this is always a required minimum)
    if (!CheckMacOSX ()) // this is false on Mac OS 9 since Mac OS 9 does not support dragging conttexts with shared txtures between to different vendor's renderers.
        glInfo.fDraggable = false; 		// should a pixel format that supports all monitors be chosen?
    else
        glInfo.fDraggable = true; 		// should a pixel format that supports all monitors be chosen?
    glInfo.fmt = 0;					// output pixel format
    
    i = 0;
    glInfo.aglAttributes [i++] = AGL_RGBA;
    glInfo.aglAttributes [i++] = AGL_DOUBLEBUFFER;
    glInfo.aglAttributes [i++] = AGL_ACCELERATED;
    glInfo.aglAttributes [i++] = AGL_NO_RECOVERY;
    glInfo.aglAttributes [i++] = AGL_DEPTH_SIZE;
    glInfo.aglAttributes [i++] = 16;
    glInfo.aglAttributes [i++] = AGL_NONE;
    
    BuildGLFromWindow (appGLWindow, &boincAGLContext, &glInfo, NULL );
    if (!boincAGLContext)
    {
        DestroyGLFromWindow (&boincAGLContext, &glInfo);
    }
    else
    {
        Rect rectPort;
        
        GetWindowPortBounds (appGLWindow, &rectPort);
        aglSetCurrentContext (boincAGLContext);
        aglReportError ();
        aglUpdateContext (boincAGLContext);
        aglReportError ();

        // Set Texture mapping parameters
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glClearColor(0.15f, 0.15f, 0.15f, 1.0f);					// Clear color buffer to dark grey
        glClear (GL_COLOR_BUFFER_BIT);
        glReportError ();
        aglSwapBuffers (boincAGLContext);
        aglReportError ();

        aglDisable (boincAGLContext, AGL_BUFFER_RECT);
        aglReportError ();
        glViewport (0, 0, rectPort.right - rectPort.left, rectPort.bottom - rectPort.top);
        glReportError ();
        
        GetFNum("\pMonaco", &fNum);									// build font
        monacoFontList = BuildFontGL (boincAGLContext, fNum, normal, 9);

        aglUpdateContext (boincAGLContext);
        aglReportError ();
    }
    
    return 0;
}

//////////////////////////////////////////////////////////////////////////////////
//  GraphicsLoopProcessor							//
//////////////////////////////////////////////////////////////////////////////////
pascal void GraphicsLoopProcessor(EventLoopTimerRef inTimer, void* timeData) {
    DoUpdate (boincAGLContext);
}

//////////////////////////////////////////////////////////////////////////////////
//  YieldProcessor								//
//////////////////////////////////////////////////////////////////////////////////
pascal void YieldProcessor(EventLoopTimerRef inTimer, void* timeData) {
    YieldToAnyThread();
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
                case kHICommandQuit:
                    QuitApplicationEventLoop();
                    result = noErr;
                    break;
                case kHICommandOK:		// 'ok  '
                case kHICommandCancel:		// 'not!'
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
                case kHICommandAbout:		// 'abou'
                    // Open About window
                    //CreateAboutWindow();
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

pascal void mac_graphics_event_loop ( GRAPHICS_INFO *gi ) {
    InitGLWindow(gi->xsize, gi->ysize, 16);
    RunApplicationEventLoop();
}

// --------------------------------------------------------------------------

void mac_cleanup (void)
{
    // TODO: Dispose all the timers here
    RemoveEventLoopTimer(boincTimer);
    DisposeEventLoopTimerUPP(boincTimerUPP);
    DisposeEventHandlerUPP(appCommandProcessor);
    DisposeGLWindow (appGLWindow);
}

// --------------------------------------------------------------------------

static void DisposeGLWindow (WindowPtr pWindow) // Dispose a single window and it's GL context
{
    if (pWindow)
    {
        DeleteFontGL (monacoFontList);
        // must clean up failure to do so will result in an Unmapped Memory Exception
        DestroyGLFromWindow (&boincAGLContext, &glInfo);
        
        DisposeWindow (pWindow);
    }
}
