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

#include <math.h>
#include <stdio.h>
#include <string.h>

// project includes ---------------------------------------------------------

#include "mac_main.h"
#include "mac_join.h"
#include "mac_about.h"
#include "mac_prefs.h"
#include "account.h"
#include "client_state.h"
#include "log_flags.h"
#include "message.h"
#include "gui_titles.h"
#include "util.h"

// statics/globals (internal only) ------------------------------------------

WindowRef           boincWindow;
EventLoopTimerRef   boincTimer,boincIdleTimer;
EventLoopTimerUPP   boincTimerUPP,boincIdleUPP;
EventHandlerUPP     appCommandProcessor,winCommandProcessor;
WindowPtr           boincAboutWindow;
Point               oldMouse;
KeyMap              oldKeys;
UInt32              user_idle_since,user_idle_length;
vector<MESSAGE*>    msgs;
ControlRef          tabControl;
int                 winXPos, winYPos, winWidth, winHeight;
int                 tabList[] = {5, 129, 130, 131, 132, 133};   // Tab UserPane IDs
int                 lastTabIndex = 1;
MenuRef             dockMenu, fileMenu;
MenuItemIndex       dockSuspendIndex, dockResumeIndex;
MenuItemIndex       fileSuspendIndex, fileResumeIndex;
bool                user_requested_exit = false;

ControlRef boincDataBrowsers[MAX_LIST_ID];

DataBrowserPropertyType column_types[MAX_LIST_ID][MAX_COLS] = {
    {kDataBrowserTextType, kDataBrowserTextType, kDataBrowserTextType, kDataBrowserTextType, kDataBrowserProgressBarType, NULL, NULL},
    {kDataBrowserTextType, kDataBrowserTextType, kDataBrowserTextType, kDataBrowserTextType, kDataBrowserProgressBarType, kDataBrowserTextType,    kDataBrowserTextType},
    {kDataBrowserTextType, kDataBrowserTextType, kDataBrowserProgressBarType, kDataBrowserTextType, kDataBrowserTextType, kDataBrowserTextType, NULL},
    {kDataBrowserTextType, kDataBrowserDateTimeType, kDataBrowserTextType, NULL, NULL, NULL, NULL}
};

DataBrowserItemDataUPP    carbonItemCallbacks[MAX_LIST_ID] =
    {BOINCCarbonProjectCallback, BOINCCarbonWorkCallback, BOINCCarbonTransferCallback, BOINCCarbonMessageCallback};

void Syncronize(ControlRef prog, vector<void*>* vect);

// --------------------------------------------------------------------------

OSStatus InitMainWindow(void) {
    OSStatus err;
    Handle boincMenuBar;
    ControlID ctrlID;
    IBNibRef boincNibRef;
    int i,n;
    DataBrowserCallbacks callbacks;
    
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
    
    ReadBOINCPreferences();
    
    // Enable the preferences item
    EnableMenuCommand(NULL, kHICommandPreferences);
    
    // Add the columns to the data browsers
    for (i=0;i<MAX_LIST_ID;i++) {
        ctrlID.signature = kBOINCDataBrowserSig;
        ctrlID.id = 128+i;
        err = GetControlByID( boincWindow, &ctrlID, &boincDataBrowsers[i] );  // Projects DataBrowser
        if (err != noErr) return err;
        for (n=0;n<MAX_COLS;n++) {
            if (strlen(g_szColumnTitles[i][n])) {
                err = AddColumnToList( boincDataBrowsers[i], g_szColumnTitles[i][n], n, column_types[i][n] );
                if (err != noErr) return err;
            }
            if (column_types[i][n] == kDataBrowserProgressBarType) {
                err = SetDataBrowserItemDataMinimum( boincDataBrowsers[i], 0 );
                err = SetDataBrowserItemDataMaximum( boincDataBrowsers[i], 100 );
            }
        }
        
        callbacks.version = kDataBrowserLatestCallbacks;
        err = InitDataBrowserCallbacks( &callbacks );
        if (err != noErr) return err;
        callbacks.u.v1.itemDataCallback = carbonItemCallbacks[i];
        err = SetDataBrowserCallbacks( boincDataBrowsers[i], &callbacks );
        if (err != noErr) return err;
    }
    
    InstallTabHandler(boincWindow);
    SelectItemOfTabControl(tabControl);
    
    //err = AddFileMenu();
    if (err != noErr) return err;
    SetMenuBar(boincMenuBar);
    SizeWindow( boincWindow, winWidth, winHeight, true );
    MoveWindow( boincWindow, winXPos, winYPos, false );
    ShowWindow(boincWindow);
    
    err = AddDockMenu();
    if (err != noErr) return err;
    SuspendBOINC(gstate.suspend_requested);
    
    // Application-level event handler installer
    appCommandProcessor = NewEventHandlerUPP(MainAppEventHandler);
    err = InstallApplicationEventHandler(appCommandProcessor, GetEventTypeCount(appEventList), appEventList, 0, NULL);
    if (err != noErr) return err;
    err = InstallMenuEventHandler(dockMenu, appCommandProcessor, GetEventTypeCount(appEventList), appEventList, 0, NULL);
    if (err != noErr) return err;
    
    // Window-level event handler installer
    winCommandProcessor = NewEventHandlerUPP(MainWinEventHandler);
    err = InstallWindowEventHandler(boincWindow, winCommandProcessor, GetEventTypeCount(winEventList), winEventList, 0, NULL);
    if (err != noErr) return err;
    
    // BOINC Timed event handler installer
    boincTimerUPP = NewEventLoopTimerUPP(BOINCPollLoopProcessor);
    err = InstallEventLoopTimer(GetCurrentEventLoop(), 0, kEventDurationMillisecond*100, boincTimerUPP, NULL, &boincTimer);
    if (err != noErr) return err;
    
    // BOINC Timed event handler installer
    boincIdleUPP = NewEventLoopTimerUPP(BOINCIdleDetect);
    err = InstallEventLoopTimer(GetCurrentEventLoop(), 0, kEventDurationMillisecond*100, boincIdleUPP, NULL, &boincIdleTimer);
    if (err != noErr) return err;
    
    DisposeNibReference(boincNibRef);
    
    return noErr;
}

OSStatus AddDockMenu( void ) {
    OSStatus  myErr;
    
    myErr = CreateNewMenu( 140, 0, &dockMenu );
    if (myErr != noErr) return myErr;
    
    myErr = AppendMenuItemTextWithCFString( dockMenu,
        CFStringCreateWithCString(CFAllocatorGetDefault(), "Suspend", kCFStringEncodingMacRoman),
        0, kBOINCCommandSuspend, &dockSuspendIndex );
    if (myErr != noErr) return myErr;
    
    myErr = AppendMenuItemTextWithCFString( dockMenu,
        CFStringCreateWithCString(CFAllocatorGetDefault(), "Resume", kCFStringEncodingMacRoman),
        0, kBOINCCommandResume, &dockResumeIndex );
    if (myErr != noErr) return myErr;
    
    myErr = SetApplicationDockTileMenu(dockMenu);
    if (myErr != noErr) return myErr;
    
    return noErr;
}

OSStatus AddFileMenu( void ) {
    OSStatus myErr;
    
    myErr = CreateNewMenu( 129, 0, &fileMenu );
    if (myErr != noErr) return myErr;
    
    myErr = SetMenuTitleWithCFString( fileMenu,
        CFStringCreateWithCString(CFAllocatorGetDefault(), "File2", kCFStringEncodingMacRoman) );
    if (myErr != noErr) return myErr;
    
    myErr = AppendMenuItemTextWithCFString( fileMenu,
        CFStringCreateWithCString(CFAllocatorGetDefault(), "Show Graphics", kCFStringEncodingMacRoman),
        0, kBOINCShowGraphics, NULL );
    if (myErr != noErr) return myErr;
    
    myErr = AppendMenuItemTextWithCFString( fileMenu,
        CFStringCreateWithCString(CFAllocatorGetDefault(), "Clear Messages", kCFStringEncodingMacRoman),
        0, kBOINCClearMessages, NULL );
    if (myErr != noErr) return myErr;
    
    myErr = AppendMenuItemTextWithCFString( fileMenu,
        CFStringCreateWithCString(CFAllocatorGetDefault(), "", kCFStringEncodingMacRoman),
        kMenuItemAttrSeparator, 0, NULL );
    if (myErr != noErr) return myErr;
    
    myErr = AppendMenuItemTextWithCFString( fileMenu,
        CFStringCreateWithCString(CFAllocatorGetDefault(), "Suspend", kCFStringEncodingMacRoman),
        0, kBOINCCommandSuspend, &fileSuspendIndex );
    if (myErr != noErr) return myErr;
    
    myErr = AppendMenuItemTextWithCFString( fileMenu,
        CFStringCreateWithCString(CFAllocatorGetDefault(), "Resume", kCFStringEncodingMacRoman),
        0, kBOINCCommandResume, &fileResumeIndex );
    if (myErr != noErr) return myErr;
    
    InsertMenu( fileMenu, 0 );
    
    return noErr;
}

OSStatus AddColumnToList( ControlRef theList, char *columnName, DataBrowserPropertyID propertyID, DataBrowserPropertyType propertyType ) {
    DataBrowserListViewColumnDesc columnDesc;
    
    columnDesc.headerBtnDesc.btnContentInfo.contentType = 0;
    columnDesc.headerBtnDesc.btnContentInfo.u.resID = 0;
    
    columnDesc.headerBtnDesc.btnFontStyle.flags = kControlUseThemeFontIDMask;
    columnDesc.headerBtnDesc.btnFontStyle.font = kControlFontViewSystemFont;
    
    columnDesc.propertyDesc.propertyID = propertyID;
    columnDesc.propertyDesc.propertyType = propertyType;
    columnDesc.propertyDesc.propertyFlags = kDataBrowserDefaultPropertyFlags;
    
    columnDesc.headerBtnDesc.version = kDataBrowserListViewLatestHeaderDesc;
    columnDesc.headerBtnDesc.minimumWidth = 0;
    columnDesc.headerBtnDesc.maximumWidth = 230;
    columnDesc.headerBtnDesc.initialOrder = kDataBrowserOrderIncreasing;  // ?
    columnDesc.headerBtnDesc.titleOffset = 0;
    
    columnDesc.headerBtnDesc.titleString = CFStringCreateWithCString(
        CFAllocatorGetDefault(), columnName, kCFStringEncodingMacRoman);
    
    return AddDataBrowserListViewColumn( theList, &columnDesc, ULONG_MAX );
}

bool CheckIfIdle( void ) {
    if (gstate.global_prefs.idle_time_to_run > 0) {
        if (user_idle_length > 60 * gstate.global_prefs.idle_time_to_run) {    // Is there a defined constant we can use instead of 60?
            return true;
        } else {
            return false;
        }
    } else {
        return true;
    }
}

////////////////////////////////////////////////////////
//  BOINCPollLoopProcessor                            //
////////////////////////////////////////////////////////
pascal void BOINCPollLoopProcessor(EventLoopTimerRef inTimer, void* timeData)
{
    gstate.user_idle = CheckIfIdle();

    // While we still have something to do we keep processing,
    // otherwise, we go back to sleep
    while (gstate.do_something()) {
        if (gstate.time_to_exit()) {
            break;
        }
    }
    
    Syncronize( boincDataBrowsers[0], (vector<void*>*)(&gstate.projects) );
    Syncronize( boincDataBrowsers[1], (vector<void*>*)(&gstate.results) );
    Syncronize( boincDataBrowsers[2], (vector<void*>*)(&gstate.pers_xfers->pers_file_xfers) );
    Syncronize( boincDataBrowsers[3], (vector<void*>*)(&msgs) );
    
    GUIRedraw();
    
    fflush(stdout);
}

/////////////////////////////////////////////////////
//  BOINCIdleDetect                                //
/////////////////////////////////////////////////////
pascal void BOINCIdleDetect(EventLoopTimerRef inTimer, void* timeData)
{
    Point curMouse;
    KeyMap curKeys;
    int i;
    UInt32 curTime;
    
    curTime = TickCount();
    
    // See if the mouse has moved
    GetMouse( &curMouse );
    if (curMouse.h != oldMouse.h || curMouse.v != oldMouse.v) {
        user_idle_since = curTime;
    }
    oldMouse.h = curMouse.h;
    oldMouse.v = curMouse.v;
    
    // See if user has pressed the mouse button
    if (Button())         
        user_idle_since = curTime;
    
    // See if any keys have been pressed
    GetKeys( curKeys );
    for (i=0;i<4;i++) {
        if (oldKeys[i] != curKeys[i]) {
            user_idle_since = curTime;
        }
        oldKeys[i] = curKeys[i];
    }
    
    user_idle_length = curTime - user_idle_since;
}

//////////////////////////////////////////////////////////////////////////////////
//  SuspendBOINC                                //
//////////////////////////////////////////////////////////////////////////////////
void SuspendBOINC( bool suspend ) {
    gstate.suspend_requested = suspend;
    if (suspend) {
        DisableMenuItem( dockMenu, dockSuspendIndex );
        DisableMenuItem( fileMenu, fileSuspendIndex );
        EnableMenuItem( dockMenu, dockResumeIndex );
        EnableMenuItem( fileMenu, fileResumeIndex );
    } else {
        DisableMenuItem( dockMenu, dockResumeIndex );
        DisableMenuItem( fileMenu, fileResumeIndex );
        EnableMenuItem( dockMenu, dockSuspendIndex );
        EnableMenuItem( fileMenu, fileSuspendIndex );
    }
}

//////////////////////////////////////////////////////////////////////////////////
//  MainAppEventHandler                                //
//////////////////////////////////////////////////////////////////////////////////
pascal OSStatus MainAppEventHandler(EventHandlerCallRef appHandler, EventRef theEvent, void* appData)
{
#pragma unused (appHandler, appData)

    HICommand    aCommand;
    OSStatus    result;

    switch(GetEventClass(theEvent))
    {
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
                case kHICommandPreferences:    // 'pref'
                    // Open prefs dialog
                    //CreatePrefsDialog();
                    result = noErr;
                    break;
                case kBOINCCommandJoin:    // 'join'
                    char new_master_url[256], new_auth[256];
                    // Open join dialog
                    if (CreateJoinDialog( new_master_url, new_auth ) ) {
                        gstate.add_project( new_master_url, new_auth );
                    }
                    result = noErr;
                    break;
                case kBOINCCommandSuspend:    // 'susp'
                    // Suspend processing
                    SuspendBOINC(true);
                    result = noErr;
                    break;
                case kBOINCCommandResume:    // 'resm'
                    // Resume processing
                    SuspendBOINC(false);
                    result = noErr;
                    break;
                case kHICommandAbout:        // 'abou'
                    // Open About window
                    CreateAboutWindow();
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


//////////////////////////////////////////////////////////////////////////////////
//  MainWinEventHandler                                //
//////////////////////////////////////////////////////////////////////////////////
pascal OSStatus MainWinEventHandler(EventHandlerCallRef appHandler, EventRef theEvent, void* appData)
{
#pragma unused (appHandler, appData)
    OSStatus    result;
    UInt32    attributes;
    Rect    winRect;

    switch(GetEventClass(theEvent))
    {
        case kEventClassWindow:
            GetEventParameter(theEvent, kEventParamAttributes, typeUInt32, NULL, sizeof(UInt32), NULL, &attributes);
            switch(GetEventKind(theEvent)) {
                case kEventWindowBoundsChanged:
                    GetEventParameter(theEvent, kEventParamCurrentBounds, typeQDRectangle, NULL, sizeof(Rect), NULL, &winRect);
                    winXPos = winRect.left;
                    winYPos = winRect.top;
                    winWidth = winRect.right - winRect.left;
                    winHeight = winRect.bottom - winRect.top;
                    if (attributes & kWindowBoundsChangeSizeChanged) {
                        SizeControl( tabControl, winWidth, winHeight-12 );
                        for (int i=0;i<4;i++ )
                            SizeControl( boincDataBrowsers[i], winWidth-30-9, winHeight-49-30 );
                    }
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

OSStatus mac_setup (void)
{
    OSStatus err;
    
    err = InitMainWindow();
    if (err != noErr) return err;
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

// --------------------------------------------------------------------------

int main(int argc, char** argv) {
    read_log_flags();
    gstate.parse_cmdline(argc, argv);

    if (gstate.init()) return -1;
    
    // mac_setup won't return until the main application loop has quit
    if (!mac_setup()) return -1;
    
    // Afterwards, we clean up and exit
    gstate.cleanup_and_exit();
    
    SaveBOINCPreferences();
    
    mac_cleanup();
}

    // Check if we're running OS 8/9 or OS X
    /*pm_features = PMFeatures();
    // If the we can get the battery info from this API, do it
    if ((pm_features>>canGetBatteryTime)&0x01) {
        err = BatteryStatus( &a, &b );
        if (err != noErr); // do something
    } else
        fprintf( stderr, "%d %d", a, b );
    exit(0);*/

// --------------------------------------------------------------------------

void show_message(PROJECT *p, char* message, int priority) {
    MESSAGE *new_msg;
    
    new_msg = (MESSAGE*)malloc(sizeof(MESSAGE));
    GetDateTime( &new_msg->timestamp );
    safe_strncpy( new_msg->msg, message, sizeof(new_msg->msg) );
    if (p)
        safe_strncpy( new_msg->project, p->get_project_name(), sizeof(new_msg->msg) );
    else
        safe_strncpy( new_msg->project, "BOINC", sizeof(new_msg->msg) );
    msgs.push_back(new_msg);
}

//
int add_new_project() {
    return 0;
}

// ----------------------------------------------------------------------
// Show the selected pane, hide the others.

void SelectItemOfTabControl(ControlRef myTabControl) {
    ControlRef userPane;
    ControlRef selectedPane = NULL;
    ControlID controlID;
    UInt16 i;

    SInt16 index = GetControlValue(myTabControl);
    
    lastTabIndex = index;
    controlID.signature = TAB_SIGNATURE;

    for (i = 1; i < tabList[0] + 1; i++) {
        controlID.id = tabList[i];
        GetControlByID(GetControlOwner(myTabControl), &controlID, &userPane);
       
        if (i == index) {
            selectedPane = userPane;
            SetControlVisibility(boincDataBrowsers[i-1],true,true);
        } else {
            SetControlVisibility(userPane,false,false);
            SetControlVisibility(boincDataBrowsers[i-1],false,false);
            DisableControl(userPane);
        }
    }
    
    if (selectedPane != NULL) {
        EnableControl(selectedPane);
        SetControlVisibility(selectedPane, true, true);
    }
    
    SizeControl( tabControl, winWidth, winHeight-12 );
    
    Draw1Control(myTabControl);
    
    for (int i=0;i<4;i++ )
        SizeControl( boincDataBrowsers[i], winWidth-30-9, winHeight-49-30 );
}

// ----------------------------------------------------------------------
// Listen to events. Only switch if the tab value has changed.

pascal OSStatus TabEventHandler(EventHandlerCallRef inHandlerRef, EventRef inEvent, void *inUserData)
{
    OSStatus result = eventNotHandledErr;
    
    ControlRef theControl;
    ControlID controlID;
    
    GetEventParameter(inEvent, kEventParamDirectObject, typeControlRef, NULL, sizeof( ControlRef ), NULL, &theControl );
    
    GetControlID(theControl, &controlID);
    
    // If this event didn't trigger a tab change, give somebody else a chance to handle it.
    if (controlID.id == TAB_ID && GetControlValue(theControl) != lastTabIndex) {
        result = noErr;
        SelectItemOfTabControl(theControl);
    }
    
    return result;
}

// ----------------------------------------------------------------------

void InstallTabHandler(WindowRef window)
{
    EventTypeSpec    controlSpec = { kEventClassControl, kEventControlHit }; // event class, event kind
    ControlID         controlID;

    // Find the tab control and install an event handler on it.
    controlID.signature = TAB_SIGNATURE;
    controlID.id = TAB_ID;
    GetControlByID(window, &controlID, &tabControl);

    InstallEventHandler(GetControlEventTarget(tabControl), NewEventHandlerUPP( TabEventHandler ),
                        1, &controlSpec, 0, NULL);

    //Select the active tab to start with.
    lastTabIndex = GetControlValue(tabControl);
    SelectItemOfTabControl(tabControl);
}

// Force a redraw of the GUI elements
//
void GUIRedraw(void) {
    DataBrowserItemID items;
    
    items = kDataBrowserNoItem;
    for (int i=0;i<MAX_LIST_ID;i++ ) {
        UpdateDataBrowserItems( boincDataBrowsers[i], kDataBrowserNoItem, 1, &items, kDataBrowserItemNoProperty, kDataBrowserNoItem );
    }
}

OSStatus BOINCCarbonProjectCallback( ControlRef browser, DataBrowserItemID itemID,
    DataBrowserPropertyID property, DataBrowserItemDataRef itemData, Boolean changeValue) {

    OSStatus err;
    char buf[256];
    unsigned int i;
    PROJECT *p = (PROJECT*)itemID;
    float totalres = 0;
    
    if (!changeValue) {
        sprintf( buf, " " );
        switch (property) {
            case 0:    // Project name
                safe_strncpy( buf, p->get_project_name(), sizeof(buf) );
                break;
            case 1:    // Account name
                safe_strncpy( buf, p->user_name, sizeof(buf) );
                break;
            case 2:    // Total credit
                sprintf( buf, "%0.2f", p->user_total_credit);
                break;
            case 3:    // Average credit
                sprintf( buf, "%0.2f", p->user_expavg_credit);
                break;
            case 4:    // Resource share
                for(i = 0; i < gstate.projects.size(); i ++) {
                    totalres += gstate.projects[i]->resource_share;
                }
                if (totalres <= 0)
                    SetDataBrowserItemDataValue( itemData, 100 );
                else
                    SetDataBrowserItemDataValue( itemData, (int)((100 * p->resource_share)/totalres) );
                break;
            default:
                break;
        }
        err = SetDataBrowserItemDataText( itemData, CFStringCreateWithCString(CFAllocatorGetDefault(), buf, kCFStringEncodingMacRoman));
    }

    return noErr;
}

OSStatus BOINCCarbonWorkCallback( ControlRef browser, DataBrowserItemID itemID,
    DataBrowserPropertyID property, DataBrowserItemDataRef itemData, Boolean changeValue) {

    OSStatus err;
    char buf[256];
    RESULT *re = (RESULT*)itemID;
    ACTIVE_TASK* at;
    int cpuhour, cpumin, cpusec;

    if (!changeValue) {
        sprintf( buf, " " );
        switch (property) {
            case 0:    // Project name
                safe_strncpy( buf, re->project->project_name, sizeof(buf) );
                break;
            case 1:    // App name
                safe_strncpy( buf, re->app->name, sizeof(buf) );
                break;
            case 2:    // Workunit name
                safe_strncpy( buf, re->name, sizeof(buf) );
                break;
            case 3:    // CPU time
                double cur_cpu;
                at = gstate.lookup_active_task_by_result(re);
                if (at) {
                    cur_cpu = at->current_cpu_time;
                } else {
                    cur_cpu = 0;
                }
                cpuhour = (int)(cur_cpu / (60 * 60));
                cpumin = (int)(cur_cpu / 60) % 60;
                cpusec = (int)(cur_cpu) % 60;
                sprintf( buf, "%02d:%02d:%02d", cpuhour, cpumin, cpusec);
                break;
            case 4:    // Progress
                at = gstate.lookup_active_task_by_result(re);
                if(!at) {
                    SetDataBrowserItemDataValue(itemData, 0);
                } else {    
                    SetDataBrowserItemDataValue(itemData, (int)(at->fraction_done * 100));
                }
                break;
            case 5:    // Time to completion
                double tocomp;
                if(!at || at->fraction_done == 0) {
                    tocomp = gstate.estimate_cpu_time(*re->wup);
                } else {
                    tocomp = at->est_time_to_completion();
                }
                cpuhour = (int)(tocomp / (60 * 60));
                cpumin = (int)(tocomp / 60) % 60;
                cpusec = (int)(tocomp) % 60;
                sprintf( buf, "%02d:%02d:%02d", cpuhour, cpumin, cpusec);
                break;
            case 6:    // Status
                switch(re->state) {
                    case RESULT_NEW:
                        safe_strncpy(buf, g_szMiscItems[0], sizeof(buf));
                        break;
                    case RESULT_FILES_DOWNLOADING:
                        safe_strncpy(buf, g_szMiscItems[9], sizeof(buf));
                        break;
                    case RESULT_FILES_DOWNLOADED:
                        if (at) safe_strncpy(buf, g_szMiscItems[1], sizeof(buf));
                        else safe_strncpy(buf, g_szMiscItems[2], sizeof(buf));
                        break;
                    case RESULT_COMPUTE_DONE:
                        safe_strncpy(buf, g_szMiscItems[3], sizeof(buf));
                        break;
                    case RESULT_FILES_UPLOADING:
                        safe_strncpy(buf, g_szMiscItems[8], sizeof(buf));
                        break;
                    default:
                        if (re->server_ack) {
                            safe_strncpy(buf, g_szMiscItems[5], sizeof(buf)); break;
                        } else if (re->ready_to_ack) {
                            safe_strncpy(buf, g_szMiscItems[4], sizeof(buf)); break;
                        } else {
                            safe_strncpy(buf, g_szMiscItems[6], sizeof(buf)); break;
                        }
                }
                break;
        }
        err = SetDataBrowserItemDataText( itemData, CFStringCreateWithCString(CFAllocatorGetDefault(), buf, kCFStringEncodingMacRoman));
    }
    
    return noErr;
}

OSStatus BOINCCarbonTransferCallback( ControlRef browser, DataBrowserItemID itemID,
    DataBrowserPropertyID property, DataBrowserItemDataRef itemData, Boolean changeValue) {

    OSStatus err;
    char buf[256];
    PERS_FILE_XFER *fi = (PERS_FILE_XFER*)itemID;
    double xSent,xtime;
    int xhour, xmin, xsec;
    
    if (!changeValue) {
        xSent = 0;
        if( fi->fip->generated_locally ) {
            xSent = fi->fip->upload_offset;
        } else {
            char pathname[256];
            get_pathname(fi->fip, pathname);
            file_size(pathname, xSent);
        }
        sprintf( buf, " " );
        switch (property) {
            case 0:    // Project name
                safe_strncpy( buf, fi->fip->project->project_name, sizeof(buf) );
                break;
            case 1:    // File name
                safe_strncpy( buf, fi->fip->name, sizeof(buf) );
                break;
            case 2:    // Transfer progress
                SetDataBrowserItemDataValue(itemData, (int)(100.0 * xSent / fi->fip->nbytes));
                break;
            case 3:    // Size
                sprintf( buf,"%0.0f/%0.0fKB", xSent / 1024.0, fi->fip->nbytes / 1024.0);
                break;
            case 4:    // Time
                xtime = fi->time_so_far;
                xhour = (int)(xtime / (60 * 60));
                xmin = (int)(xtime / 60) % 60;
                xsec = (int)(xtime) % 60;
                sprintf(buf,"%02d:%02d:%02d", xhour, xmin, xsec);
                break;
            case 5:    // Status
                if (fi->next_request_time > time(0)) {
                    xtime = fi->next_request_time-time(0);
                    xhour = (int)(xtime / (60 * 60));
                    xmin = (int)(xtime / 60) % 60;
                    xsec = (int)(xtime) % 60;
                    sprintf(buf,"%s %02d:%02d:%02d", g_szMiscItems[10], xhour, xmin, xsec);
                } else if (fi->fip->status == ERR_GIVEUP_DOWNLOAD) {
                    safe_strncpy( buf, g_szMiscItems[11], sizeof(buf) );
                } else if (fi->fip->status == ERR_GIVEUP_UPLOAD) {
                    safe_strncpy( buf, g_szMiscItems[12], sizeof(buf) );
                } else {
                    safe_strncpy( buf, fi->fip->generated_locally?g_szMiscItems[8]:g_szMiscItems[9], sizeof(buf) );
                }
                break;
        }
        err = SetDataBrowserItemDataText( itemData, CFStringCreateWithCString(CFAllocatorGetDefault(), buf, kCFStringEncodingMacRoman));
    }

    return noErr;
}

OSStatus BOINCCarbonMessageCallback( ControlRef browser, DataBrowserItemID itemID,
    DataBrowserPropertyID property, DataBrowserItemDataRef itemData, Boolean changeValue) {

    OSStatus err;
    char buf[256];
    MESSAGE *msg = (MESSAGE*)itemID;
    
    if (!changeValue) {
        switch (property) {
            case 0:    // Project
                safe_strncpy( buf, msg->project, sizeof(buf) );
                err = SetDataBrowserItemDataText( itemData, CFStringCreateWithCString(CFAllocatorGetDefault(), buf, kCFStringEncodingMacRoman));
                break;
            case 1:    // Time
                SetDataBrowserItemDataDateTime( itemData, msg->timestamp );
                break;
            case 2:    // File name
                safe_strncpy( buf, msg->msg, sizeof(buf) );
                err = SetDataBrowserItemDataText( itemData, CFStringCreateWithCString(CFAllocatorGetDefault(), buf, kCFStringEncodingMacRoman));
                break;
            default:
                safe_strncpy( buf, " ", sizeof(buf) );
                err = SetDataBrowserItemDataText( itemData, CFStringCreateWithCString(CFAllocatorGetDefault(), buf, kCFStringEncodingMacRoman));
                break;
        }
    }

    return noErr;
}

//////////
// arguments:    prog: pointer to a progress list control
//                vect: pointer to a vector of pointers
// returns:        void
// function:    first, goes through the vector and adds items to the list
//                control for any pointers it does not already contain, then
//                goes through the list control and removes any pointers the
//                vector does not contain.
void Syncronize(ControlRef prog, vector<void*>* vect) {
    unsigned int i,j;
    Handle items;
    OSStatus err;
    DataBrowserItemID duff[1];
    
    items = NewHandle(0);
    err = GetDataBrowserItems(prog, kDataBrowserNoItem, true, 0, items);

    // add items to list that are not already in it
    for(i = 0; i < vect->size(); i ++) {
        void* item = (*vect)[i];
        bool contained = false;
        for(j = 0; j < (GetHandleSize(items) / sizeof(DataBrowserItemID)); j ++) {
            if(item == items[j]) {
                contained = true;
                break;
            }
        }
        if(!contained) {
            duff[0] = (long unsigned int)item;
            err = AddDataBrowserItems( prog, kDataBrowserNoItem, 1, duff, kDataBrowserItemNoProperty );
        }
    }

    // remove items from list that are not in vector
    // now just set the pointer to NULL but leave the item in the list
    for(i = 0; i < (GetHandleSize(items) / sizeof(DataBrowserItemID)); i ++) {
        void* item = items[i];
        bool contained = false;
        for(j = 0; j < vect->size(); j ++) {
            if(item == (*vect)[j]) {
                contained = true;
                break;
            }
        }
        if(!contained) {
            duff[0] = (long unsigned int)item;
            err = RemoveDataBrowserItems( prog, kDataBrowserNoItem, 1, duff, kDataBrowserItemNoProperty );
        }
    }
}

OSStatus SaveBOINCPreferences( void ) {
    unsigned int i, n;
    char buf[256];
    UInt32 ncols;
    
    // Set up the preferences
    CFPreferencesSetAppValue(CFSTR("mainWindowXPos"), CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &winXPos),
        kCFPreferencesCurrentApplication);
    CFPreferencesSetAppValue(CFSTR("mainWindowYPos"), CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &winYPos),
        kCFPreferencesCurrentApplication);
    CFPreferencesSetAppValue(CFSTR("mainWindowWidth"), CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &winWidth),
        kCFPreferencesCurrentApplication);
    CFPreferencesSetAppValue(CFSTR("mainWindowHeight"), CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &winHeight),
        kCFPreferencesCurrentApplication);
    CFPreferencesSetAppValue(CFSTR("mainWindowCurTab"), CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &lastTabIndex),
        kCFPreferencesCurrentApplication);
    
    for (i=0;i<4;i++) {
        GetDataBrowserTableViewColumnCount(boincDataBrowsers[i],&ncols);
        for (n=0;n<ncols;n++) {
            sprintf( buf, "table-%d-col-%d", i, n );
            CFPreferencesSetAppValue(CFStringCreateWithCString(kCFAllocatorDefault, buf, kCFStringEncodingMacRoman),
                CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &i),
                kCFPreferencesCurrentApplication);
        }
    }
    
    CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);
    
    return noErr;
}

OSStatus ReadBOINCPreferences( void ) {
    Boolean validPref;
    
    winXPos = CFPreferencesGetAppIntegerValue( CFSTR("mainWindowXPos"), kCFPreferencesCurrentApplication, &validPref );
    if (!validPref) { // also check if xPos is out of bounds
        winXPos = 100;
    }
    winYPos = CFPreferencesGetAppIntegerValue( CFSTR("mainWindowYPos"), kCFPreferencesCurrentApplication, &validPref );
    if (!validPref) { // also check if yPos is out of bounds
        winYPos = 100;
    }
    winWidth = CFPreferencesGetAppIntegerValue( CFSTR("mainWindowWidth"), kCFPreferencesCurrentApplication, &validPref );
    if (!validPref) {
        winWidth = 640;
    }
    winHeight = CFPreferencesGetAppIntegerValue( CFSTR("mainWindowHeight"), kCFPreferencesCurrentApplication, &validPref );
    if (!validPref) {
        winHeight = 480;
    }
    lastTabIndex = CFPreferencesGetAppIntegerValue( CFSTR("mainWindowCurTab"), kCFPreferencesCurrentApplication, &validPref );
    if (!validPref) {
        lastTabIndex = 1;
    }
    
    return 0;
}

void delete_curtain() {
}

void create_curtain() {
}

void project_add_failed(PROJECT* project) {
}

const char *BOINC_RCSID_f788d5cc4e = "$Id$";
