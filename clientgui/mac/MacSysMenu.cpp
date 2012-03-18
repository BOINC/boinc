// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "MacSysMenu.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "BOINCBaseFrame.h"
#include "MainDocument.h"
#include "MacSysMenu.h"
#include "DlgAbout.h"
#include "Events.h"
#include "miofile.h"
#include "SkinManager.h"
#include "wx/mac/private.h"     // for wxBitmapRefData::GetPictHandle

pascal OSStatus SysMenuEventHandler( EventHandlerCallRef inHandlerCallRef,
                                    EventRef inEvent, void* pData);

    EventTypeSpec myEvents[] = { {kEventClassCommand, kEventCommandProcess},
                                    { kEventClassApplication, kEventAppHidden},
                                    { kEventClassApplication, kEventAppShown},
                                    {kEventClassMenu, kEventMenuOpening} };


#if wxCHECK_VERSION(2,8,0)

// Class declarations copied from wxMac-2.8.0/src/mac/carbon/taskbar.cpp
// We had to copy these because they are not in a header file.

class wxTaskBarIconImpl
{
public:
    wxTaskBarIconImpl(wxTaskBarIcon* parent);
    virtual ~wxTaskBarIconImpl();

    virtual bool IsIconInstalled() const = 0;
    virtual bool SetIcon(const wxIcon& icon) = 0;
    virtual bool RemoveIcon() = 0;
    virtual bool PopupMenu(wxMenu *menu) = 0;

    wxMenu * CreatePopupMenu()
    { return m_parent->CreatePopupMenu(); }

    wxTaskBarIcon *m_parent;
    class wxTaskBarIconWindow *m_menuEventWindow;

    DECLARE_NO_COPY_CLASS(wxTaskBarIconImpl)
};

//-----------------------------------------------------------------------------
//
//  wxTaskBarIconWindow
//
//  Event handler for menus
//  NB: Since wxWindows in Mac HAVE to have parents we need this to be
//  a top level window...
//-----------------------------------------------------------------------------

class wxTaskBarIconWindow : public wxTopLevelWindow
{
public:
    wxTaskBarIconWindow(wxTaskBarIconImpl *impl)
        : wxTopLevelWindow(NULL, wxID_ANY, wxEmptyString), m_impl(impl)
    {
        Connect(
            -1, wxEVT_COMMAND_MENU_SELECTED,
            wxCommandEventHandler(wxTaskBarIconWindow::OnMenuEvent) );
    }

    void OnMenuEvent(wxCommandEvent& event)
    {
        m_impl->m_parent->ProcessEvent(event);
    }

private:
    wxTaskBarIconImpl *m_impl;
};

class wxDockTaskBarIcon : public wxTaskBarIconImpl
{
public:
    wxDockTaskBarIcon(wxTaskBarIcon* parent);
    virtual ~wxDockTaskBarIcon();

    virtual bool IsIconInstalled() const;
    virtual bool SetIcon(const wxIcon& icon);
    virtual bool RemoveIcon();
    virtual bool PopupMenu(wxMenu *menu);

    wxMenu* DoCreatePopupMenu();

    EventHandlerRef     m_eventHandlerRef;
    EventHandlerUPP     m_eventupp;
    wxWindow           *m_eventWindow;
    wxMenu             *m_pMenu;
    MenuRef             m_theLastMenu;
    bool                m_iconAdded;
};

#endif


CMacSystemMenu::CMacSystemMenu(wxString title, wxIcon* icon, wxIcon* iconDisconnected, wxIcon* iconSnooze)
                                : CTaskBarIcon(title, icon, iconDisconnected, iconSnooze) {
     CFBundleRef	SysMenuBundle	= NULL;

    m_bOpeningAboutDlg = false;
    m_bNeedRebuildMenu = false;
    
    LoadPrivateFrameworkBundle( CFSTR("SystemMenu.bundle"), &SysMenuBundle );
    if ( SysMenuBundle != NULL )
    {
        SetUpSystemMenu = (SetUpSystemMenuProc) 
                            CFBundleGetFunctionPointerForName( SysMenuBundle, CFSTR("SetUpSystemMenu") );
        SetSystemMenuIcon = (SetSystemMenuIconProc) 
                            CFBundleGetFunctionPointerForName( SysMenuBundle, CFSTR("SetSystemMenuIcon") );
        
        BuildMenu();

        // The base class wxTaskBarIcon will install the wxDockEventHandler for 
        // each instance of the derived classes CTaskBarIcon and CMacSystemMenu.
        // We remove that handler and substitute our own.
#if wxCHECK_VERSION(2,8,0)
        RemoveEventHandler((EventHandlerRef&)(((wxDockTaskBarIcon*)m_impl)->m_eventHandlerRef));
        
        InstallApplicationEventHandler(NewEventHandlerUPP(SysMenuEventHandler), 
                                sizeof(myEvents) / sizeof(EventTypeSpec), myEvents, 
                                                        this, (EventHandlerRef*)&(((wxDockTaskBarIcon*)m_impl)->m_eventHandlerRef)); 
#else
        RemoveEventHandler((EventHandlerRef&)m_pEventHandlerRef);

        InstallApplicationEventHandler(NewEventHandlerUPP(SysMenuEventHandler), 
                                sizeof(myEvents) / sizeof(EventTypeSpec), myEvents, 
                                                        this, (EventHandlerRef*)&m_pEventHandlerRef); 
#endif
    }
}


CMacSystemMenu::~CMacSystemMenu() {
    // Remove the System Menu (StatusItem) from menu bar
    if (SetSystemMenuIcon != NULL )
        SetSystemMenuIcon(NULL);
}


// Set the System Menu Icon from XPM data
bool CMacSystemMenu::SetIcon(const wxIcon& icon) {
    wxBitmap theBits;

    // For unknown reasons, menus won't work if we call BuildMenu() directly 
    // from CTaskBarIcon::OnReloadSkin(), so it sets a flag to call it here
    if (m_bNeedRebuildMenu) {
        CMainDocument*     pDoc = wxGetApp().GetDocument();
        wxASSERT(pDoc);
        wxASSERT(wxDynamicCast(pDoc, CMainDocument));
        if (pDoc->IsConnected() && m_bNeedRebuildMenu) {
            // For unknown reasons, Menubar Icon menu doesn't work without this
            BuildMenu();
        }
    }
    m_bNeedRebuildMenu = false;
    
    theBits.CopyFromIcon(icon);
    CGImageRef imageRef = (CGImageRef)theBits.CGImageCreate();
    if ( (SetSystemMenuIcon != NULL) && (imageRef != NULL) ) { 
        SetSystemMenuIcon(imageRef);
        CGImageRelease( imageRef );
        return true;
    }
    
    if(imageRef != NULL) CGImageRelease( imageRef );
                
    return false;
}


void CMacSystemMenu::BuildMenu() {
    wxBitmap theBits;
    wxMenu *themenu;
    CSkinAdvanced* pSkinAdvanced = wxGetApp().GetSkinManager()->GetAdvanced();

    wxASSERT(pSkinAdvanced);
    wxASSERT(wxDynamicCast(pSkinAdvanced, CSkinAdvanced));

    m_iconTaskBarNormal = *pSkinAdvanced->GetApplicationIcon();
    m_iconTaskBarDisconnected = *pSkinAdvanced->GetApplicationDisconnectedIcon();
    m_iconTaskBarSnooze = *pSkinAdvanced->GetApplicationSnoozeIcon();

    theBits.CopyFromIcon(m_iconTaskBarNormal);
    CGImageRef imageRef = (CGImageRef)theBits.CGImageCreate();                
    if ( (SetUpSystemMenu != NULL ) && (imageRef != NULL) ) {
        // Currently, the system menu is the same as the Dock menu with the addition of 
        // the Quit menu item.  If in the future you wish to make the system menu different 
        // from the Dock menu, override CTaskBarIcon::BuildContextMenu() and 
        // CTaskBarIcon::AdjustMenuItems().
#if wxCHECK_VERSION(2,8,0)
        delete ((wxDockTaskBarIcon*)m_impl)->m_pMenu;
        ((wxDockTaskBarIcon*)m_impl)->m_pMenu = BuildContextMenu();
        themenu = ((wxDockTaskBarIcon*)m_impl)->m_pMenu;
#else
        delete m_pMenu;
        m_pMenu = BuildContextMenu();
        themenu = m_pMenu;
#endif

        // These should appear in the Mac System Menu but not the Dock
        themenu->AppendSeparator();
        themenu->Append( wxID_EXIT, _("E&xit"), wxEmptyString );
        
        themenu->SetEventHandler(this);

        SetUpSystemMenu((MenuRef)(themenu->GetHMenu()), imageRef);
    }
    if(imageRef != NULL) CGImageRelease( imageRef );
}


#if wxCHECK_VERSION(2,8,0)

wxMenu* CMacSystemMenu::GetCurrentMenu() {
    return ((wxDockTaskBarIcon*)m_impl)->m_pMenu;
}

#endif


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
             if (!pMSM) break;
                
           // wxMac "helpfully" converts wxID_ABOUT to kHICommandAbout, wxID_EXIT to kHICommandQuit, 
            //  wxID_PREFERENCES to kHICommandPreferences
            switch (commandID) {
            case kHICommandAbout:
                commandID = wxID_ABOUT;
                break;
            case kHICommandPreferences:
                {
                    CBOINCBaseFrame* pFrame = wxGetApp().GetFrame();
                    wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, ID_PREFERENCES);
                    pFrame->AddPendingEvent(evt);
                    return noErr ;
                }
            case kHICommandQuit:
                if (wxGetApp().ConfirmExit()) {
                    commandID = wxID_EXIT;
                } else {
                    commandID = 0;
                    return noErr;
                }
                break;
            }
                
            if (commandID == wxID_ABOUT)
                pMSM->SetOpeningAboutDlg(true);
                
            // If not our system menu, pass event on to next event handler
           if (command.menu.menuRef != (MenuRef)'BNC!') {           // Used only in OS 10.5
                if (PLstrcmp("\pBOINC!", (GetMenuTitle((command.menu.menuRef), theMenuTitle) ))) {
                    return eventNotHandledErr;
                }
            }
            
            // The following code is adapted from wxTaskBarIcon's wxDockEventHandler().
            // Work with our base menu instead of the cloned System Menu
            baseMenu = (pMSM->GetCurrentMenu());
            baseMenuRef = (MenuRef)(baseMenu->GetHMenu());
        
            err = GetIndMenuItemWithCommandID(baseMenuRef, command.commandID, 1, NULL, &menuItemIndex);
            if (err)
                return eventNotHandledErr;  // Command not found in our menu
                
            GetMenuItemRefCon( baseMenuRef, menuItemIndex, (UInt32*) &item ) ;

            if ( item )
            {
                if (item->IsCheckable())
                    item->Check( !item->IsChecked() );

                item->GetMenu()->SendEvent( commandID , item->IsCheckable() ? item->IsChecked() : -1 );
                
                // Under wxWidgets 2.8.0, the task bar icons must be deleted for app to 
                // exit its main loop
                // Note that if the main window is open, CBOINCBaseFrame::OnExit() will be 
                // called and SysMenuEventHandler() (i.e., this code) will not be called.
                if (commandID == wxID_EXIT) {
                    wxGetApp().DeleteTaskBarIcon();
                }
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
                GetMenuItemText(baseMenuRef, i, theMenuTitle);
                SetMenuItemText(sysMenuRef, i, theMenuTitle);
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
            CBOINCBaseFrame* pFrame = wxGetApp().GetFrame();
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
