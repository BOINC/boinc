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

#include <Carbon/Carbon.h>

#ifdef __cplusplus
extern "C" {
#endif

void InitMainWindow(void);
void DisplayBOINCStatusWindow (int left, int top, int width, int height);
pascal OSStatus MainAppEventHandler(EventHandlerCallRef appHandler, EventRef theEvent, void* appData);
pascal void BOINCPollLoopProcessor(EventLoopTimerRef inTimer, void* timeData);
int mac_setup (void);
void mac_cleanup (void);

#ifdef __cplusplus
}
#endif
