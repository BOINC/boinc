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
// $Log$
// Revision 1.2  2004/09/24 02:01:53  rwalton
// *** empty log message ***
//
// Revision 1.1  2004/09/21 01:26:25  rwalton
// *** empty log message ***
//
//

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "ViewResources.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "ViewResources.h"
#include "Events.h"

#include "res/usage.xpm"


IMPLEMENT_DYNAMIC_CLASS(CViewResources, CBOINCBaseView)

BEGIN_EVENT_TABLE(CViewResources, CBOINCBaseView)
END_EVENT_TABLE()


CViewResources::CViewResources()
{
    wxLogTrace("CViewResources::CViewResources - Function Begining");
    wxLogTrace("CViewResources::CViewResources - Function Ending");
}


CViewResources::CViewResources(wxNotebook* pNotebook) :
    CBOINCBaseView(pNotebook, ID_HTML_RESOURCEUTILIZATIONVIEW, ID_LIST_RESOURCEUTILIZATIONVIEW)
{
    m_bProcessingRenderEvent = false;

    wxASSERT(NULL != m_pTaskPane);
    wxASSERT(NULL != m_pListPane);

}


CViewResources::~CViewResources()
{
    wxLogTrace("CViewResources::~CViewResources - Function Begining");
    wxLogTrace("CViewResources::~CViewResources - Function Ending");
}


wxString CViewResources::GetViewName()
{
    return wxString(_("Disk"));
}


char** CViewResources::GetViewIcon()
{
    return usage_xpm;
}


void CViewResources::OnRender(wxTimerEvent &event)
{
}

