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

#include "mac_app_opengl.h"
#include "mac_carbon_gl.h"

#ifdef __APPLE_CC__
#else
    #include <Devices.h>
    #include <Dialogs.h>
    #include <DriverServices.h>
    #include <Events.h>
    #include <Gestalt.h>
    #include <LowMem.h>
    #include <TextEdit.h>
    #include <ToolUtils.h>
    #include <QDOffscreen.h>
    #include <Windows.h>
#endif

#include "boinc_gl.h"

#include <cstdio>
#include <cstring>

// project includes ---------------------------------------------------------

#include "app_ipc.h"
#include "graphics_api.h"

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
EventLoopTimerRef     boincTimer;
EventLoopTimerUPP    boincTimerUPP;
EventHandlerUPP     appCommandProcessor,winCommandProcessor;
AGLContext        boincAGLContext;
GLuint            main_font;
structGLWindowInfo    glInfo;

bool user_requested_exit = false;
extern bool using_opengl;

// --------------------------------------------------------------------------

int InitGLWindow(int xsize, int ysize, int depth, double refresh_period) {
    OSStatus err;
    Rect winRect;
    TimerUPP boincYieldUPP;
    EventLoopTimerRef boincYieldTimer;
    short i,fNum;

    InitCursor();

    SetRect( &winRect, 100, 100, 100+xsize, 100+ysize );

    err = CreateNewWindow ( kDocumentWindowClass, kWindowStandardDocumentAttributes, &winRect, &appGLWindow );
    if (err != noErr) return -1;

    // Application-level event handler installer
    appCommandProcessor = NewEventHandlerUPP(MainAppEventHandler);
    err = InstallApplicationEventHandler(appCommandProcessor, GetEventTypeCount(appEventList),
                                            appEventList, 0, NULL);

    // Window-level event handler installer
    /*winCommandProcessor = NewEventHandlerUPP(MainWinEventHandler);
    err = InstallWindowEventHandler(appGLWindow, 0,
                                kEventDurationMillisecond*10,        // Every 10 ms
                                boincYieldUPP, NULL, &boincYieldTimer);*/


    // Install preemption
    boincYieldUPP = NewEventLoopTimerUPP(YieldProcessor);
    err = InstallEventLoopTimer(GetMainEventLoop(), 0,
                                kEventDurationMillisecond*10,        // Every 10 ms
                                boincYieldUPP, NULL, &boincYieldTimer);

    // Install graphics
    boincYieldUPP = NewEventLoopTimerUPP(GraphicsLoopProcessor);
    err = InstallEventLoopTimer(GetMainEventLoop(), 0,
                                kEventDurationMillisecond*refresh_period*1000,
                                boincYieldUPP, NULL, &boincYieldTimer);

    // TODO: add an event handler for the window
    ChangeWindowAttributes( appGLWindow, kWindowStandardHandlerAttribute, 0 );
    SetWTitle (appGLWindow, "\pWindow");
    ShowWindow(appGLWindow);
    SetPortWindowPort (appGLWindow);

    glInfo.fmt = 0;                    // output pixel format

    i = 0;
    glInfo.aglAttributes [i++] = AGL_RGBA;
    glInfo.aglAttributes [i++] = AGL_DOUBLEBUFFER;
    glInfo.aglAttributes [i++] = AGL_ACCELERATED;
    glInfo.aglAttributes [i++] = AGL_NO_RECOVERY;
    glInfo.aglAttributes [i++] = AGL_DEPTH_SIZE;
    glInfo.aglAttributes [i++] = 16;
    glInfo.aglAttributes [i++] = AGL_NONE;

    BuildGLonWindow (appGLWindow, &boincAGLContext, &glInfo);
    if (!boincAGLContext) {
        DestroyGLFromWindow (&boincAGLContext, &glInfo);
    } else {
        Rect rectPort;

        GetWindowPortBounds (appGLWindow, &rectPort);
        aglSetCurrentContext (boincAGLContext);
        aglReportError ();
        aglUpdateContext (boincAGLContext);
        aglReportError ();

        aglDisable (boincAGLContext, AGL_BUFFER_RECT);
        aglReportError ();
        //ReSizeGLScene( rectPort.right - rectPort.left, rectPort.bottom - rectPort.top );
        glReportError ();

        app_graphics_init();

        glClear (GL_COLOR_BUFFER_BIT);
        glReportError ();
        aglSwapBuffers (boincAGLContext);
        aglReportError ();

        GetFNum("\pTimes New Roman", &fNum);                                    // build font
        main_font = BuildFontGL (boincAGLContext, fNum, normal, 9);

        aglUpdateContext (boincAGLContext);
        aglReportError ();

        using_opengl = true;
    }

    return 0;
}

//////////////////////////////////////////////////////////////////////////////////
//  GraphicsLoopProcessor                            //
//////////////////////////////////////////////////////////////////////////////////
pascal void GraphicsLoopProcessor(EventLoopTimerRef inTimer, void* timeData) {
    DoUpdate (boincAGLContext);
    YieldToAnyThread();
}

//////////////////////////////////////////////////////////////////////////////////
//  YieldProcessor                                //
//////////////////////////////////////////////////////////////////////////////////
pascal void YieldProcessor(EventLoopTimerRef inTimer, void* timeData) {
    YieldToAnyThread();
}

//////////////////////////////////////////////////////////////////////////////////
//  MainAppEventHandler                                //
//////////////////////////////////////////////////////////////////////////////////
pascal OSStatus MainAppEventHandler(EventHandlerCallRef appHandler, EventRef theEvent, void* appData)
{
#pragma unused (appHandler, appData)

    HICommand    aCommand;
    OSStatus    result;
    Point    mDelta;
    Rect rectPort;

    switch(GetEventClass(theEvent))
    {
        case kEventClassMouse:                // 'mous'
            GetEventParameter(theEvent,            // the event itself
                              kEventParamMouseDelta,     // symbolic parameter name
                              typeQDPoint,         // expected type
                              NULL,             // actual type (NULL is valid)
                              sizeof(mDelta),         // buffer size
                              NULL,             // actual buffer size (Can be NULL)
                              &mDelta);            // variable to hold data
            switch(GetEventKind(theEvent))
            {
                case kEventMouseDown:
                case kEventMouseUp:
                case kEventMouseMoved:
                case kEventMouseDragged:
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
                    //QuitApplicationEventLoop();
                    HideWindow(appGLWindow);
                    result = noErr;
                    break;
                case kHICommandMaximizeWindow:    // 'mini'
                case kHICommandZoomWindow:    // 'zoom'
                    GetWindowPortBounds (appGLWindow, &rectPort);
                    //ReSizeGLScene(rectPort.right - rectPort.left, rectPort.bottom - rectPort.top);
                    //InitGL();
                    app_graphics_init();
                    glReportError ();
                    break;
                case kHICommandHide:        // 'hide'
                case kHICommandMinimizeWindow:    // 'mini'
                case kHICommandArrangeInFront:    // 'frnt'
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

pascal void *mac_graphics_event_loop () {
    InitGLWindow(600, 400, 16, 1./30.);
    RunApplicationEventLoop();
    return NULL;
}

// --------------------------------------------------------------------------

void mac_cleanup (void) {
    // TODO: Dispose all the timers here
    RemoveEventLoopTimer(boincTimer);
    DisposeEventLoopTimerUPP(boincTimerUPP);
    DisposeEventHandlerUPP(appCommandProcessor);
    DisposeGLWindow (appGLWindow);
}

// --------------------------------------------------------------------------

void DisposeGLWindow (WindowPtr pWindow) {  // Dispose a single window and it's GL context
    if (pWindow) {
        DeleteFontGL (main_font);
        // must clean up failure to do so will result in an Unmapped Memory Exception
        DestroyGLFromWindow (&boincAGLContext, &glInfo);
        DisposeWindow (pWindow);
    }
}
