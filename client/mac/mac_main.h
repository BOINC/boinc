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
