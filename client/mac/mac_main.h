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

#include <Carbon/Carbon.h>
#include "error_numbers.h"
#include "filesys.h"

#define kBOINCDataBrowserSig    'duff'

#define TAB_ID          128
#define TAB_SIGNATURE    'tabs'

enum {
    kBOINCCommandJoin = 'join',
    kBOINCShowGraphics = 'sgfx',
    kBOINCClearMessages = 'clms',
    kBOINCCommandQuitProject = 'qprj',
    kBOINCCommandSuspend = 'susp',
    kBOINCCommandResume = 'resu'
};

extern OSStatus InitMainWindow(void);
extern OSStatus AddDockMenu( void );
extern OSStatus AddFileMenu( void );
extern OSStatus AddColumnToList( ControlRef theList, char *columnName, DataBrowserPropertyID propertyID, DataBrowserPropertyType propertyType );
extern bool CheckIfIdle (void);
extern pascal void BOINCPollLoopProcessor(EventLoopTimerRef inTimer, void* timeData);
extern pascal void BOINCIdleDetect(EventLoopTimerRef inTimer, void* timeData);
extern void SuspendBOINC( bool suspend );
extern pascal OSStatus MainAppEventHandler(EventHandlerCallRef appHandler, EventRef theEvent, void* appData);
extern pascal OSStatus MainWinEventHandler(EventHandlerCallRef appHandler, EventRef theEvent, void* appData);
extern OSStatus mac_setup (void);
extern void mac_cleanup (void);
extern void SelectItemOfTabControl(ControlRef myTabControl);
extern pascal OSStatus TabEventHandler(EventHandlerCallRef inHandlerRef, EventRef inEvent, void *inUserData);
extern void InstallTabHandler(WindowRef window);
extern void GUIRedraw(void);
extern OSStatus BOINCCarbonProjectCallback( ControlRef browser, DataBrowserItemID itemID,
    DataBrowserPropertyID property, DataBrowserItemDataRef itemData, Boolean changeValue);
extern OSStatus BOINCCarbonWorkCallback( ControlRef browser, DataBrowserItemID itemID,
    DataBrowserPropertyID property, DataBrowserItemDataRef itemData, Boolean changeValue);
extern OSStatus BOINCCarbonTransferCallback( ControlRef browser, DataBrowserItemID itemID,
    DataBrowserPropertyID property, DataBrowserItemDataRef itemData, Boolean changeValue);
extern OSStatus BOINCCarbonMessageCallback( ControlRef browser, DataBrowserItemID itemID,
    DataBrowserPropertyID property, DataBrowserItemDataRef itemData, Boolean changeValue);
extern OSStatus SaveBOINCPreferences( void );
extern OSStatus ReadBOINCPreferences( void );

//void DisplayBOINCStatusWindow (int left, int top, int width, int height);

static const EventTypeSpec  appEventList[] = { {kEventClassCommand, kEventCommandProcess} };

static const EventTypeSpec  winEventList[] = { {kEventClassWindow, kEventWindowBoundsChanged} };

typedef struct MESSAGE {
    char project[256];
    char msg[256];
    UInt32 timestamp;
};

typedef struct MESSAGE MESSAGE;
