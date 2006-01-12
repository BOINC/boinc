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

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "MacSysMenu.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "MacSysMenu.h"
#include "DlgAbout.h"
#include "Events.h"
#include "wx/mac/private.h"     // for wxBitmapRefData::GetPictHandle

pascal OSStatus SysMenuEventHandler( EventHandlerCallRef inHandlerCallRef,
                                    EventRef inEvent, void* pData);

    EventTypeSpec myEvents[] = { {kEventClassCommand, kEventCommandProcess},
                                    { kEventClassApplication, kEventAppHidden},
                                    { kEventClassApplication, kEventAppShown},
                                    {kEventClassMenu, kEventMenuOpening} };

CMacSystemMenu::CMacSystemMenu(wxString title, wxIcon* icon) : CTaskBarIcon(title, icon) {
    CFBundleRef	SysMenuBundle	= NULL;
    wxBitmapRefData * theBitsRefData;
    PicHandle thePICT;
    wxBitmap theBits;

    theBits.CopyFromIcon(*icon);
    theBitsRefData = theBits.GetBitmapData();
    thePICT = theBitsRefData->GetPictHandle();

    m_OpeningAboutDlg = false;
    
    LoadPrivateFrameworkBundle( CFSTR("SystemMenu.bundle"), &SysMenuBundle );
    if ( SysMenuBundle != NULL )
    {
        SetUpSystemMenu = (SetUpSystemMenuProc) 
                            CFBundleGetFunctionPointerForName( SysMenuBundle, CFSTR("SetUpSystemMenu") );
        SetSystemMenuIcon = (SetSystemMenuIconProc) 
                            CFBundleGetFunctionPointerForName( SysMenuBundle, CFSTR("SetSystemMenuIcon") );
        if ( (SetUpSystemMenu != NULL ) && (thePICT != NULL) )
        {
            // Currently, the system menu is the same as the Dock menu with the addition of 
            // the Quit menu item.  If in the future you wish to make the system menu different 
            // from the Dock menu, override CTaskBarIcon::BuildContextMenu() and 
            // CTaskBarIcon::AdjustMenuItems().
            delete m_pMenu;
            m_pMenu = BuildContextMenu();
            
            // These should appear in the Mac System Menu but not the Dock
            m_pMenu->AppendSeparator();
            m_pMenu->Append( wxID_EXIT, _("E&xit"), wxEmptyString );
            
            m_pMenu->SetEventHandler(this);
            SetUpSystemMenu((MenuRef)(m_pMenu->GetHMenu()), thePICT);
        }

        // The base class wxTaskBarIcon will install the wxDockEventHandler for 
        // each instance of the derived classes CTaskBarIcon and CMacSystemMenu.
        // We remove that handler and substitute our own.
        RemoveEventHandler((EventHandlerRef&)m_pEventHandlerRef);

        InstallApplicationEventHandler(NewEventHandlerUPP(SysMenuEventHandler), 
                                sizeof(myEvents) / sizeof(EventTypeSpec), myEvents, 
                                                        this, (EventHandlerRef*)&m_pEventHandlerRef); 
    }
}


CMacSystemMenu::~CMacSystemMenu() {
}


// Set the System Menu Icon from XPM data
// Warning - this code has not been tested
bool CMacSystemMenu::SetIcon(const wxIcon& icon, const wxString&) {
    wxBitmapRefData * theBitsRefData;
    PicHandle thePICT;
    wxBitmap theBits;

    theBits.CopyFromIcon(icon);
    theBitsRefData = theBits.GetBitmapData();
    thePICT = theBitsRefData->GetPictHandle();
    if ( (SetSystemMenuIcon != NULL ) && (thePICT != NULL) ) {
        SetSystemMenuIcon(thePICT);
        return true;
    }
    return false;
}


//	Utility routine to load a bundle from the application's Frameworks folder.
//	i.e. : "BOINC.app/Contents/Frameworks/SystemMenu.bundle"
void CMacSystemMenu::LoadPrivateFrameworkBundle( CFStringRef framework, CFBundleRef *bundlePtr ) {
	CFURLRef	baseURL			= NULL;
	CFURLRef	bundleURL		= NULL;
	CFBundleRef	myAppsBundle	= NULL;

	if ( bundlePtr == NULL )	goto Bail;
	*bundlePtr = NULL;
	
	myAppsBundle	= CFBundleGetMainBundle();					//	Get our application's main bundle from Core Foundation
	if ( myAppsBundle == NULL )	goto Bail;
	
	baseURL	= CFBundleCopyPrivateFrameworksURL( myAppsBundle );
	if ( baseURL == NULL )	goto Bail;

	bundleURL = CFURLCreateCopyAppendingPathComponent( kCFAllocatorSystemDefault, baseURL, framework, false );
	if ( bundleURL == NULL )	goto Bail;

	*bundlePtr = CFBundleCreate( kCFAllocatorSystemDefault, bundleURL );
	if ( *bundlePtr == NULL )	goto Bail;

	if ( ! CFBundleLoadExecutable( *bundlePtr ) )
	{
		CFRelease( *bundlePtr );
		*bundlePtr	= NULL;
	}

Bail:															// Clean up.
	if ( bundleURL != NULL )	CFRelease( bundleURL );
	if ( baseURL != NULL )		CFRelease( baseURL );
}


pascal OSStatus SysMenuEventHandler( EventHandlerCallRef inHandlerCallRef,
                                    EventRef inEvent, void* pData) {
    HICommand command;
    MenuRef baseMenuRef, sysMenuRef;
    Str255 theMenuTitle;
    short i, n, m;
    CharParameter markChar;
    const UInt32 eventClass = GetEventClass(inEvent);
    const UInt32 eventKind = GetEventKind(inEvent);
    CMacSystemMenu* pMSM;
    wxMenu* baseMenu;
    MenuCommand commandID;
    MenuItemIndex menuItemIndex;
    wxMenuItem* item = NULL;
    OSStatus err;

    switch (eventClass) {
        case kEventClassCommand:
            if (eventKind != kEventCommandProcess)
                return eventNotHandledErr;
            
            GetEventParameter (inEvent, kEventParamDirectObject, 
                                    typeHICommand, NULL, sizeof(HICommand), NULL, &command);

            commandID = command.commandID;
            if (commandID == 0)
                return eventNotHandledErr;

            pMSM = wxGetApp().GetMacSystemMenu();
                
            if (commandID == wxID_ABOUT)
                pMSM->SetOpeningAboutDlg(true);
                
            // If not our system menu, pass event on to next event handler
            sysMenuRef = command.menu.menuRef;
            if (PLstrcmp("\pBOINC!", (GetMenuTitle((sysMenuRef), theMenuTitle) )))
                return eventNotHandledErr;

            // The following code is adapted from wxTaskBarIcon's wxDockEventHandler().
            // Work with our base menu instead of the cloned System Menu
            baseMenu = (pMSM->GetCurrentMenu());
            baseMenuRef = (MenuRef)(baseMenu->GetHMenu());
        
            err = GetIndMenuItemWithCommandID(baseMenuRef, commandID, 1, NULL, &menuItemIndex);
            if (err)
                return eventNotHandledErr;  // Command not found in our menu
                
            GetMenuItemRefCon( baseMenuRef, menuItemIndex, (UInt32*) &item ) ;

            if ( item )
            {
                if (item->IsCheckable())
                    item->Check( !item->IsChecked() );

                item->GetMenu()->SendEvent( commandID , item->IsCheckable() ? item->IsChecked() : -1 );
                return noErr ;
            }

            return eventNotHandledErr;

        case kEventClassMenu:
            if (eventKind != kEventMenuOpening)
                return eventNotHandledErr;

            GetEventParameter (inEvent, kEventParamDirectObject, 
                        typeMenuRef, NULL, sizeof(sysMenuRef), NULL, &sysMenuRef);

            // If not our system menu, pass event on to next event handler
            if (PLstrcmp("\pBOINC!", (GetMenuTitle((sysMenuRef), theMenuTitle) )))
                return eventNotHandledErr;

            pMSM = wxGetApp().GetMacSystemMenu();
            baseMenu = (pMSM->GetCurrentMenu());
            pMSM->AdjustMenuItems(baseMenu);
            
            // Copy checkmark and enabled status of each item from Dock menu
            baseMenuRef = (MenuRef)(baseMenu->GetHMenu());
            n = CountMenuItems(sysMenuRef);
            m = CountMenuItems(baseMenuRef);
            if (m < n)
                n = m;
            for (i=1; i<=n; i++)
            {
                GetMenuItemCommandID(baseMenuRef, i, &commandID);
                SetMenuItemCommandID(sysMenuRef, i, commandID);
                GetItemMark(baseMenuRef, i, &markChar);
                SetItemMark(sysMenuRef, i, markChar);
                if( IsMenuItemEnabled(baseMenuRef, i) )
                    EnableMenuItem(sysMenuRef, i);
                else
                    DisableMenuItem(sysMenuRef, i);
            }

            return noErr;

        // Event handling to open or close our main window when applicaiton 
        // is shown or hidden.  This probably should go in BOINCGUIApp.cpp 
        // or MainFrame.cpp, but it is more eficient to put it here since 
        // we already have this Mac Event Handler installed.
        case kEventClassApplication:
            CMainFrame* pFrame = wxGetApp().GetFrame();
            pMSM = wxGetApp().GetMacSystemMenu();
            switch (eventKind) {
                case kEventAppHidden:
                    if (pFrame)
                        pFrame->Hide();
                        pMSM->SetOpeningAboutDlg(false);
                    break;

                case kEventAppShown:
                    // Don't open main window if "About" Dialog from task bar menu. 
                    if (pMSM->IsOpeningAboutDlg()) {
                        pMSM->SetOpeningAboutDlg(false);
                        break;
                    }
                    if (ActiveNonFloatingWindow())  // Prevent infinite loop
                        break;
                    if (pFrame) {
                        pFrame->Show();
                        pFrame->SendSizeEvent();
                    }
                    // If a modal dialog was open, make sure it remains in front
                    WindowRef win = GetWindowList();
                    WindowModality modality = kWindowModalityNone;
                    while (win) {
                        GetWindowModality(win, &modality, NULL);
                        if (modality == kWindowModalityAppModal)
                            BringToFront(win);
                        win = GetNextWindow(win);
                    }
                    break;
            }
            
            return eventNotHandledErr;
            
    }   // End switch (eventClass)
    
    return eventNotHandledErr;
}
const char *BOINC_RCSID_533878e385="$Id$";
