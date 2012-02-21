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
#pragma implementation "sg_ClientStateIndicator.h"
#endif

#include "stdwx.h"
#include "diagnostics.h"
#include "util.h"
#include "mfile.h"
#include "miofile.h"
#include "parse.h"
#include "error_numbers.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "MainDocument.h"
#include "BOINCBaseFrame.h"
#include "sg_BoincSimpleGUI.h"
#include "sg_ImageLoader.h"
#include "sg_ClientStateIndicator.h" 
#include "time.h"

#define ID_ANIMATIONRENDERTIMER  12000

IMPLEMENT_DYNAMIC_CLASS(ClientStateIndicator, wxPanel)

BEGIN_EVENT_TABLE(ClientStateIndicator, wxPanel) 
        EVT_PAINT(ClientStateIndicator::OnPaint)
		EVT_ERASE_BACKGROUND(ClientStateIndicator::OnEraseBackground)
		EVT_TIMER(ID_ANIMATIONRENDERTIMER, ClientStateIndicator::RunConnectionAnimation)
END_EVENT_TABLE() 

ClientStateIndicator::ClientStateIndicator() {}

ClientStateIndicator::ClientStateIndicator(CSimplePanel* parent,wxPoint coord) : 
    wxPanel(parent, wxID_ANY, coord, wxSize(343,314), wxNO_BORDER) 
{
	m_connRenderTimer = NULL;
	connIndicatorWidth = 14;
	connIndicatorHeight = 15;
	numOfIndic = 5;
	indexIndVis = 1;//first will be visible on start
	rightPosition = 118;
	topPosition = 5;
	stateMessage = wxEmptyString;
	clientState = CLIENT_STATE_NONE;
	CreateComponent();
	error_time = 0;
}

ClientStateIndicator::~ClientStateIndicator()
{
   if (m_connRenderTimer) {
        m_connRenderTimer->Stop();
        delete m_connRenderTimer;
    }
}

void ClientStateIndicator::CreateComponent(){
    CSkinSimple* pSkinSimple = wxGetApp().GetSkinManager()->GetSimple();

    wxASSERT(pSkinSimple);
    wxASSERT(wxDynamicCast(pSkinSimple, CSkinSimple));

	//Set Background color
    SetBackgroundColour(*pSkinSimple->GetBackgroundImage()->GetBackgroundColor());
}


void ClientStateIndicator::SetActionState(wxString message)
{
    CSkinSimple* pSkinSimple = wxGetApp().GetSkinManager()->GetSimple();

    wxASSERT(pSkinSimple);
    wxASSERT(wxDynamicCast(pSkinSimple, CSkinSimple));

	Freeze();
	stateMessage = message;
	if ( clientState != CLIENT_STATE_ACTION ) {
		//Delete Previous state
		DeletePreviousState();

		clientState = CLIENT_STATE_ACTION;
		i_indBg = new ImageLoader(this);
		i_indBg->Move(wxPoint(42,74));
        i_indBg->LoadImage(*(pSkinSimple->GetStateIndicatorBackgroundImage()->GetBitmap()));

		for(int x = 0; x < numOfIndic; x++){
			ImageLoader *i_connInd = new ImageLoader(this);
			i_connInd->Move(wxPoint(rightPosition +(connIndicatorWidth+10) * x,84));
            i_connInd->LoadImage(*(pSkinSimple->GetConnectingIndicatorImage()->GetBitmap()));
			if(x !=0){
				i_connInd->Show(false);
			}
			m_connIndV.push_back(i_connInd);
		}
		//set animation timer for interface
		if ( !m_connRenderTimer ) {
			m_connRenderTimer = new wxTimer(this, ID_ANIMATIONRENDERTIMER);
		}
		m_connRenderTimer->Start(500); 
	}
    Thaw();
}

void ClientStateIndicator::SetPausedState(wxString message)
{
    CSkinSimple* pSkinSimple = wxGetApp().GetSkinManager()->GetSimple();

    wxASSERT(pSkinSimple);
    wxASSERT(wxDynamicCast(pSkinSimple, CSkinSimple));

	Freeze();
	stateMessage = message;
	if ( clientState != CLIENT_STATE_PAUSED ) {
		//Delete Previous state
		DeletePreviousState();

		clientState = CLIENT_STATE_PAUSED;
		i_indBg = new ImageLoader(this);
		i_indBg->Move(wxPoint(42,74));
        i_indBg->LoadImage(*(pSkinSimple->GetStateIndicatorBackgroundImage()->GetBitmap()));

	
		for(int x = 0; x < numOfIndic; x++){
			ImageLoader *i_connInd = new ImageLoader(this);
			i_connInd->Move(wxPoint(rightPosition +(connIndicatorWidth+10) * x,84));
            i_connInd->LoadImage(*(pSkinSimple->GetConnectingIndicatorImage()->GetBitmap()));
			m_connIndV.push_back(i_connInd);
		}
	}
    Thaw();
}
void ClientStateIndicator::SetNoActionState(wxString message)
{
    CSkinSimple* pSkinSimple = wxGetApp().GetSkinManager()->GetSimple();

    wxASSERT(pSkinSimple);
    wxASSERT(wxDynamicCast(pSkinSimple, CSkinSimple));

	Freeze();
	stateMessage = message; 

	if ( clientState != CLIENT_STATE_ERROR ) {
		//Delete Previous state
		DeletePreviousState();

		clientState = CLIENT_STATE_ERROR;
		i_indBg = new ImageLoader(this);
		i_indBg->Move(wxPoint(42,74));
        i_indBg->LoadImage(*(pSkinSimple->GetStateIndicatorBackgroundImage()->GetBitmap()));

		i_errorInd = new ImageLoader(this);
		i_errorInd->Move(wxPoint(rightPosition+24,84));
        i_errorInd->LoadImage(*(pSkinSimple->GetErrorIndicatorImage()->GetBitmap()));
		i_errorInd->Refresh();
	}
	Thaw();	
}
void ClientStateIndicator::DeletePreviousState()
{
	if(clientState == CLIENT_STATE_ACTION || clientState == CLIENT_STATE_PAUSED){
		if (m_connRenderTimer && clientState == CLIENT_STATE_ACTION) {
			m_connRenderTimer->Stop();
			delete m_connRenderTimer;
			m_connRenderTimer = NULL;
		}
		for(int indIndex = 0; indIndex < numOfIndic; indIndex++){
			delete m_connIndV.at(indIndex);
		}
		//clear vector
		if(m_connIndV.size() > 0){
			m_connIndV.clear();
		}
		//delete ind bg
		delete i_indBg;
	}else if(clientState == CLIENT_STATE_ERROR){
		delete i_errorInd;
		//delete ind bg
		delete i_indBg;
	}
}
void ClientStateIndicator::RunConnectionAnimation(wxTimerEvent& WXUNUSED(event)){
	
	if(indexIndVis < numOfIndic){
		indexIndVis++;
		for(int j = 0; j < indexIndVis; j++){
			ImageLoader *currInd = m_connIndV[j];
			currInd->Show(true);
		}
	}else{
		indexIndVis = 0;
		for(int i = 0; i < numOfIndic; i++){
			ImageLoader *currInd = m_connIndV[i];
			currInd->Show(false);
		}
	}
}
void ClientStateIndicator::ReskinInterface()
{
	DisplayState();
}
void ClientStateIndicator::OnPaint(wxPaintEvent& WXUNUSED(event)) 
{ 
    wxPaintDC dc(this);
	//set font
	dc.SetFont(wxFont(9,74,90,90,0,wxT("Arial")));
	// center the text
	wxCoord height, width;
	dc.GetTextExtent(stateMessage, &width, &height);
	dc.DrawText(stateMessage, wxPoint(176-width/2,120)); 
     	
}
void ClientStateIndicator::OnEraseBackground(wxEraseEvent& event){
    CSkinSimple* pSkinSimple = wxGetApp().GetSkinManager()->GetSimple();

    wxASSERT(pSkinSimple);
    wxASSERT(wxDynamicCast(pSkinSimple, CSkinSimple));

	event.Skip(false);
	wxDC *dc;
	dc=event.GetDC();
	dc->SetBackground(wxBrush(this->GetBackgroundColour(),wxSOLID));
	dc->Clear();
    dc->DrawBitmap(*(pSkinSimple->GetWorkunitAreaBackgroundImage()->GetBitmap()), 0, 0); 

}

bool ClientStateIndicator::DownloadingResults() {
	bool return_value = false;
	CMainDocument* pDoc     = wxGetApp().GetDocument();
	if ( pDoc->results.results.size() > 0 ) {
		RESULT* result;
		for(unsigned int i=0; !return_value && i < pDoc->results.results.size(); i++ ) {
			result = pDoc->result(i);
			if ( result != NULL && result->state == RESULT_FILES_DOWNLOADING ) {
				return_value = true;
			}
		}
	}
	return return_value;
}

bool ClientStateIndicator::Suspended() {
	CMainDocument* pDoc     = wxGetApp().GetDocument();
	CC_STATUS status;
	bool result = false;
	pDoc->GetCoreClientStatus(status);
	if ( pDoc->IsConnected() && status.task_suspend_reason > 0 && status.task_suspend_reason != SUSPEND_REASON_DISK_SIZE &&  status.task_suspend_reason != SUSPEND_REASON_CPU_THROTTLE ) {
		result = true;
	}
	return result;
}

// Check to see if a project update is scheduled or in progress
bool ClientStateIndicator::ProjectUpdateScheduled() {
	CMainDocument* pDoc     = wxGetApp().GetDocument();
	PROJECT* project;
	int prjCount = pDoc->GetSimpleProjectCount();
	for(int i=0; i<prjCount; i++) {
		project = pDoc->state.projects[i];
		if ( project->sched_rpc_pending || project->master_url_fetch_pending || project->scheduler_rpc_in_progress ) {
			return true;
		}
	}
	return false;
}

void ClientStateIndicator::DisplayState() {
	CMainDocument* pDoc     = wxGetApp().GetDocument();
	if ( pDoc->IsReconnecting() ) {
		error_time = 0;
		SetActionState(_("Retrieving current status."));
	} else if ( pDoc->IsConnected() && pDoc->state.projects.size() == 0) {
		error_time = 0;
		SetPausedState(_("You don't have any projects.  Please Add a Project."));
	} else if ( DownloadingResults() ) {
		error_time = 0;
		SetActionState(_("Downloading work from the server."));
	} else if ( Suspended() ) {
		CC_STATUS status;
		pDoc->GetCoreClientStatus(status);
		if ( status.task_suspend_reason & SUSPEND_REASON_BATTERIES ) {
			SetActionState(_("Processing Suspended:  Running On Batteries."));
		} else if ( status.task_suspend_reason & SUSPEND_REASON_USER_ACTIVE ) {
			SetActionState(_("Processing Suspended:  User Active."));
		} else if ( status.task_suspend_reason & SUSPEND_REASON_USER_REQ ) {
			SetActionState(_("Processing Suspended:  User paused processing."));
		} else if ( status.task_suspend_reason & SUSPEND_REASON_TIME_OF_DAY ) {
			SetActionState(_("Processing Suspended:  Time of Day."));
		} else if ( status.task_suspend_reason & SUSPEND_REASON_BENCHMARKS ) {
			SetActionState(_("Processing Suspended:  Benchmarks Running."));
		} else {
			SetActionState(_("Processing Suspended."));
		}
	} else if ( ProjectUpdateScheduled() ) {
		error_time = 0;
		SetActionState(_("Waiting to contact project servers."));
	} else {
		if ( error_time == 0 ) {
			error_time = time(NULL) + 10;
			SetActionState(_("Retrieving current status"));
		} else if ( error_time < time(NULL) ) {
			if ( pDoc->IsConnected() ) {
				SetNoActionState(_("No work available to process"));
			} else {
				SetNoActionState(_("Unable to connect to the core client"));
			}
		} else {
			SetActionState(_("Retrieving current status"));
		}
	}
}
