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
#pragma implementation "sg_DlgMessages.h"
#endif

#include "stdwx.h"
#include "common_defs.h"
#include "diagnostics.h"
#include "util.h"
#include "mfile.h"
#include "miofile.h"
#include "parse.h"
#include "error_numbers.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "MainDocument.h"
#include "sg_DlgMessages.h"
#include "sg_SGUIListControl.h"
#include "Events.h"

enum 
{ 
    ID_CLOSEBUTTON = 20001,
}; 

#define COLUMN_PROJECT              0
#define COLUMN_TIME                 1
#define COLUMN_MESSAGE              2

BEGIN_EVENT_TABLE( CDlgMessages,wxDialog)
  EVT_BUTTON(-1,CDlgMessages::OnBtnClick)
END_EVENT_TABLE()
// end events

CDlgMessages::CDlgMessages(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
{
	Create(parent,id,title,pos,size,style,name);

	if((pos==wxDefaultPosition)&&(size==wxDefaultSize)){
		SetSize(0,0,545,450);
	}

	if((pos!=wxDefaultPosition)&&(size==wxDefaultSize)){
		SetSize(545,450);
	}

#ifdef __WXMAC__
    SetSize(544,450);
#endif

    m_pBackgroundPanel = new CPanelMessages(this);
    Centre();
}

CDlgMessages::~CDlgMessages()
{
}

void CDlgMessages::OnBtnClick(wxCommandEvent& /*event*/){ //init function
    EndModal(wxID_CANCEL);
} //end function


BEGIN_EVENT_TABLE( CPanelMessages,wxPanel)
  EVT_TIMER(ID_REFRESHMESSAGESTIMER, CPanelMessages::OnListRender)
  EVT_ERASE_BACKGROUND(CPanelMessages::OnEraseBackground)
END_EVENT_TABLE()
// end events

CPanelMessages::CPanelMessages(wxWindow* parent) : 
    wxPanel(parent, -1, wxDefaultPosition, wxDefaultSize, wxNO_BORDER)
{
	m_bProcessingListRenderEvent = false;

    wxWindow* pWin = wxDynamicCast(GetParent(), wxWindow);
    wxASSERT(pWin);

    SetSize(pWin->GetSize());

    m_pList = new CSGUIListCtrl(this, ID_SIMPLE_MESSAGESVIEW, DEFAULT_LIST_MULTI_SEL_FLAGS);
    wxASSERT(m_pList);

	// Create List Pane Items
    m_pList->InsertColumn(COLUMN_PROJECT, _("Project"), wxLIST_FORMAT_LEFT, 109);
    m_pList->InsertColumn(COLUMN_TIME, _("Time"), wxLIST_FORMAT_LEFT, 130);
    m_pList->InsertColumn(COLUMN_MESSAGE, _("Message"), wxLIST_FORMAT_LEFT, 250);

	m_pMessageInfoAttr = new wxListItemAttr(*wxBLACK, *wxWHITE, wxNullFont);
    m_pMessageErrorAttr = new wxListItemAttr(*wxRED, *wxWHITE, wxNullFont);

	initBefore();
	//Create dialog
	CreateDialog();
	initAfter();
}

CPanelMessages::~CPanelMessages()
{
	if (m_pRefreshMessagesTimer) {
        m_pRefreshMessagesTimer->Stop();
        delete m_pRefreshMessagesTimer;
    }
	if (m_pMessageInfoAttr) {
        delete m_pMessageInfoAttr;
        m_pMessageInfoAttr = NULL;
    }

    if (m_pMessageErrorAttr) {
        delete m_pMessageErrorAttr;
        m_pMessageErrorAttr = NULL;
    }

}


void CPanelMessages::CreateDialog()
{
    CSkinSimple* pSkinSimple = wxGetApp().GetSkinManager()->GetSimple();

    wxASSERT(pSkinSimple);
    wxASSERT(wxDynamicCast(pSkinSimple, CSkinSimple));

	//Set Background color
    SetBackgroundColour(*pSkinSimple->GetBackgroundImage()->GetBackgroundColor());
	
	wxToolTip *ttClose = new wxToolTip(_("Close message window"));
    btnClose=new wxBitmapButton(this,ID_CLOSEBUTTON,*pSkinSimple->GetCloseButton()->GetBitmap(),wxPoint(472,398),wxSize(57,16),wxBU_AUTODRAW);
	if ( pSkinSimple->GetCloseButton()->GetBitmapClicked() != NULL ) {
		btnClose->SetBitmapSelected(*pSkinSimple->GetCloseButton()->GetBitmapClicked());
	}
	btnClose->SetToolTip(ttClose);
	
	Refresh();
}

void CPanelMessages::VwXDrawBackImg(wxEraseEvent& event,wxWindow *win,wxBitmap* bitMap,int opz){
 event.Skip(false);wxDC *dc;
 dc=event.GetDC();
 dc->SetBackground(wxBrush(win->GetBackgroundColour(),wxSOLID));
 dc->Clear();
 switch (opz) {
  case 0:{
         dc->DrawBitmap(*bitMap, 0, 0);
         break;}
  case 1:{
         wxRect rec=win->GetClientRect();
         rec.SetLeft((rec.GetWidth()-bitMap->GetWidth())   / 2);
         rec.SetTop ((rec.GetHeight()-bitMap->GetHeight()) / 2);
         dc->DrawBitmap(*bitMap,rec.GetLeft(),rec.GetTop(),0);
         break;}
  case 2:{
         wxRect rec=win->GetClientRect();
         for(int y=0;y < rec.GetHeight();y+=bitMap->GetHeight()){
           for(int x=0;x < rec.GetWidth();x+=bitMap->GetWidth()){
             dc->DrawBitmap(*bitMap,x,y,0);
           }
         }
         break;}
 }
}
void CPanelMessages::OnEraseBackground(wxEraseEvent& event){
    CSkinSimple* pSkinSimple = wxGetApp().GetSkinManager()->GetSimple();

    wxASSERT(pSkinSimple);
    wxASSERT(wxDynamicCast(pSkinSimple, CSkinSimple));
 wxObject *m_wxWin = event.GetEventObject();

 if(m_wxWin==this){
     event.Skip(true);
     VwXDrawBackImg(
         event,
         this,
         pSkinSimple->GetMessagesDialogBackgroundImage()->GetBitmap(),
         0
     );
     VwXEvOnEraseBackground(event) ;
     return;
 }
 event.Skip(true);
}

void CPanelMessages::VwXEvOnEraseBackground(wxEraseEvent& WXUNUSED(event)){ //init function
} //end function

void CPanelMessages::initBefore(){
}

void CPanelMessages::initAfter(){
    //add your code here
	//set polling timer for interface
	m_pRefreshMessagesTimer = new wxTimer(this, ID_REFRESHMESSAGESTIMER);
    wxASSERT(m_pRefreshMessagesTimer);
    m_pRefreshMessagesTimer->Start(1000);  
}

wxInt32 CPanelMessages::GetDocCount() {
    return wxGetApp().GetDocument()->GetMessageCount();
}
void CPanelMessages::OnListRender (wxTimerEvent& event) {
    if (!m_bProcessingListRenderEvent) {
        m_bProcessingListRenderEvent = true;

        wxASSERT(m_pList);

        wxInt32 iDocCount = GetDocCount();
        if (0 >= iDocCount) {
            m_pList->DeleteAllItems();
        } else {
            if (m_iPreviousDocCount != iDocCount)
                m_pList->SetItemCount(iDocCount);
        }

        if ((iDocCount) && (_EnsureLastItemVisible()) && (m_iPreviousDocCount != iDocCount)) {
            m_pList->EnsureVisible(iDocCount - 1);
        }

        if (m_iPreviousDocCount != iDocCount) {
            m_iPreviousDocCount = iDocCount;
        }

        m_bProcessingListRenderEvent = false;
    }

    event.Skip();
}

wxString CPanelMessages::OnListGetItemText(long item, long column) const {
    wxString        strBuffer   = wxEmptyString;

    switch(column) {
    case COLUMN_PROJECT:
        FormatProjectName(item, strBuffer);
        break;
    case COLUMN_TIME:
        FormatTime(item, strBuffer);
        break;
    case COLUMN_MESSAGE:
        FormatMessage(item, strBuffer);
        break;
    }

    return strBuffer;
}
wxListItemAttr* CPanelMessages::OnListGetItemAttr(long item) const {
    wxListItemAttr* pAttribute  = NULL;
    wxString        strBuffer   = wxEmptyString;

	FormatPriority(item, strBuffer);

    if (wxT("E") == strBuffer) {
        pAttribute = m_pMessageErrorAttr;
    } else {
        pAttribute = m_pMessageInfoAttr;
    }

    return pAttribute;
	
}
bool CPanelMessages::_EnsureLastItemVisible() {
    return EnsureLastItemVisible();
}
bool CPanelMessages::EnsureLastItemVisible() {
    return true;
}
wxInt32 CPanelMessages::FormatProjectName(wxInt32 item, wxString& strBuffer) const {
    MESSAGE* message = wxGetApp().GetDocument()->message(item);

    if (message) {
        strBuffer = wxString(message->project.c_str(), wxConvUTF8);
    }

    return 0;
}


wxInt32 CPanelMessages::FormatPriority(wxInt32 item, wxString& strBuffer) const {
    MESSAGE* message = wxGetApp().GetDocument()->message(item);

    if (message) {
        switch(message->priority) {
        case MSG_INFO:
            strBuffer = wxT("I");
            break;
        default:
            strBuffer = wxT("E");
            break;
        }
    }

    return 0;
}


wxInt32 CPanelMessages::FormatTime(wxInt32 item, wxString& strBuffer) const {
    wxDateTime dtBuffer;
    MESSAGE*   message = wxGetApp().GetDocument()->message(item);

    if (message) {
        dtBuffer.Set((time_t)message->timestamp);
        strBuffer = dtBuffer.Format();
    }

    return 0;
}
wxInt32 CPanelMessages::FormatMessage(wxInt32 item, wxString& strBuffer) const {
    MESSAGE*   message = wxGetApp().GetDocument()->message(item);

    if (message) {
        strBuffer = wxString(message->body.c_str(), wxConvUTF8);
    }

    strBuffer.Replace(wxT("\n"), wxT(""), true);

    return 0;
}
