// $Id$
//
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
// Revision History:
//

#ifndef _TASKBARICON_H_
#define _TASKBARICON_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "TaskBarIcon.cpp"
#endif


class CTaskBarIcon : public wxTaskBarIcon
{
    DECLARE_DYNAMIC_CLASS(CTaskBarIcon)

public:
    CTaskBarIcon();
    ~CTaskBarIcon();

    enum ICONTYPES
    {
#ifdef __WXMSW__
        Info = NIIF_INFO,
        Warning = NIIF_WARNING,
        Error = NIIF_ERROR
#endif
    };

    void OnExit( wxCommandEvent& event );
    void OnClose( wxCloseEvent& event );

    virtual void OnMouseMove( wxEvent& event );
    virtual void OnRButtonDown( wxEvent& event );
    virtual void OnLButtonDClick( wxEvent& event );

    bool ShowBalloon( 
        wxString title, 
        wxString message, 
        unsigned int timeout = 5000, 
        ICONTYPES icon = ICONTYPES::Info
    );

private:

    wxIcon     iconTaskBarIcon;
    wxDateTime dtLastMouseCaptureTime;

    DECLARE_EVENT_TABLE()

};


#endif

