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

#ifdef __APPLE__
#include "../res/boinc_mac.xpm"
#else
#include "../res/boinc.xpm"
#endif

pascal OSStatus SysMenuEventHandler( EventHandlerCallRef inHandlerCallRef,
                                    EventRef inEvent, void* pData);

    EventTypeSpec myEvents[] = { {kEventClassCommand, kEventCommandProcess},
                                    { kEventClassApplication, kEventAppHidden},
                                    { kEventClassApplication, kEventAppShown},
                                    {kEventClassMenu, kEventMenuOpening} };

    EventTypeSpec removeEventList[] = { { kEventClassApplication, kEventAppGetDockTileMenu } };

IMPLEMENT_DYNAMIC_CLASS(CMacSystemMenu, CTaskBarIcon)

CMacSystemMenu::CMacSystemMenu() : CTaskBarIcon()
{
    CFBundleRef	SysMenuBundle	= NULL;
    wxBitmapRefData * theBitsRefData;
    PicHandle thePICT;

    wxBitmap theBits = wxBitmap(boinc_xpm);
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
        // But we don't want our instance of wxDockEventHandler to respond to 
        // user clicks on the Dock icon, because our wxDockEventHandler would then 
        // call CTaskBarIcon::CreatePopupMenu(), replacing the menu we passed 
        // to SetUpSystemMenu.  To prevent this, remove kEventAppGetDockTileMenu 
        // from the list of events which trigger our instance of wxDockEventHandler.
        // wxDockEventHandler is in wx_mac-2.5.x/src/mac/carbon/taskbar.cpp.
        RemoveEventTypesFromHandler((EventHandlerRef&)m_pEventHandlerRef, 1, removeEventList);

        InstallApplicationEventHandler(NewEventHandlerUPP(SysMenuEventHandler), 
                                sizeof(myEvents) / sizeof(EventTypeSpec), myEvents, 
                                                        this, &m_pSysMenuEventHandlerRef); 
    }
}


CMacSystemMenu::~CMacSystemMenu()
{
        RemoveEventHandler((EventHandlerRef&)m_pSysMenuEventHandlerRef);
}


// Set the System Menu Icon from XPM data
void CMacSystemMenu::SetIcon(const char **iconData)
{
    wxBitmapRefData * theBitsRefData;
    PicHandle thePICT;
    
    wxBitmap theBits = wxBitmap(iconData);
    theBitsRefData = theBits.GetBitmapData();
    thePICT = theBitsRefData->GetPictHandle();
    if ( (SetSystemMenuIcon != NULL ) && (thePICT != NULL) )
        SetSystemMenuIcon(thePICT);
}


//	Utility routine to load a bundle from the application's Frameworks folder.
//	i.e. : "BOINC.app/Contents/Frameworks/SystemMenu.bundle"
void CMacSystemMenu::LoadPrivateFrameworkBundle( CFStringRef framework, CFBundleRef *bundlePtr )
{
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
                                    EventRef inEvent, void* pData)
{
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

    switch (eventClass) {
        case kEventClassCommand:
            if (eventKind != kEventCommandProcess)
                return eventNotHandledErr;
            
            GetEventParameter (inEvent, kEventParamDirectObject, 
                                    typeHICommand, NULL, sizeof(HICommand), NULL, &command);

            // Special case for system icon menu
            if ((command.attributes == kHICommandFromMenu) && 
                                                    (command.commandID == 0))
                return eventNotHandledErr;

            pMSM = wxGetApp().GetMacSystemMenu();
                
            if (command.commandID == wxID_ABOUT)
                pMSM->SetOpeningAboutDlg(true);
                
            // If not our system menu, pass event on to next event handler
//            sysMenuRef = command.menu.menuRef;
            if (PLstrcmp("\pBOINC!", (GetMenuTitle((sysMenuRef), theMenuTitle) )))
                return eventNotHandledErr;

            // Change the command to point to the same item in our base (prototype) 
            //  menu and pass the event on to the Dock's menu event handler.
            baseMenu = (pMSM->GetCurrentMenu());
            baseMenuRef = (MenuRef)(baseMenu->GetHMenu());
            command.menu.menuRef = baseMenuRef;
            
            SetEventParameter((EventRef) inEvent, kEventParamDirectObject, 
                    typeHICommand, sizeof(typeHICommand), &command);

            SetEventParameter((EventRef) inEvent, kEventParamMenuRef, 
                    typeMenuRef, sizeof(MenuRef), &baseMenuRef);
                    
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
                    break;
            }
            
            return eventNotHandledErr;
            
    }   // End switch (eventClass)
    
    return eventNotHandledErr;
}
