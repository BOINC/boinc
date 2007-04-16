/*
 *  mac_prefs.h
 *  boinc
 *
 *  Created by Eric Heien on Mon Aug 12 2002.
 *  Copyright (c) 2002 __MyCompanyName__. All rights reserved.
 *
 */

#include <Carbon/Carbon.h>

OSStatus CreatePrefsDialog();
pascal OSStatus PrefsDialogEventHandler (EventHandlerCallRef myHandler, EventRef event, void *userData);
