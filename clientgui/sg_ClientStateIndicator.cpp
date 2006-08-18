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
#pragma implementation "sg_ClientStateIndicator.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "sg_BoincSimpleGUI.h"
#include "sg_SkinClass.h"
#include "sg_ImageLoader.h"
#include "sg_ClientStateIndicator.h" 

#define ID_ANIMATIONRENDERTIMER  12000

IMPLEMENT_DYNAMIC_CLASS(ClientStateIndicator, wxPanel)

BEGIN_EVENT_TABLE(ClientStateIndicator, wxPanel) 
        EVT_PAINT(ClientStateIndicator::OnPaint)
		EVT_ERASE_BACKGROUND(ClientStateIndicator::OnEraseBackground)
		EVT_TIMER(ID_ANIMATIONRENDERTIMER, ClientStateIndicator::RunConnectionAnimation)
END_EVENT_TABLE() 

ClientStateIndicator::ClientStateIndicator() {}

ClientStateIndicator::ClientStateIndicator(CSimpleFrame* parent,wxPoint coord) : wxPanel(parent, wxID_ANY, coord, wxSize(343,314), wxNO_BORDER) 
{
	connIndicatorWidth = 14;
	connIndicatorHeight = 15;
	numOfIndic = 3;
	indexIndVis = 1;//first will be visible on start
	rightPosition = 142;
	topPosition = 5;
	stateMessage = wxString("");
	clientCurrState = "";
	LoadSkinImages();	
	CreateComponent();
}

ClientStateIndicator::~ClientStateIndicator()
{
	if (m_connRenderTimer) {
        m_connRenderTimer->Stop();
        delete m_connRenderTimer;
    }
}

void ClientStateIndicator::LoadSkinImages()
{
	//app skin class
	appSkin = SkinClass::Instance();
	wxString dirPref = appSkin->GetSkinsFolder()+_T("/")+appSkin->GetSkinName()+_T("/");
	// comp bg
	g_compBg = new wxImage(dirPref + appSkin->GetWorkunitBg(), wxBITMAP_TYPE_PNG);
	m_compBG = wxBitmap(g_compBg); 
	//state ind bg
    g_stateIndBg = new wxImage(dirPref + appSkin->GetStateIndBg(), wxBITMAP_TYPE_PNG);
	m_stateIndBG = wxBitmap(g_stateIndBg); 
	// indicators
	g_connInd = new wxImage(dirPref + appSkin->GetConnInd(), wxBITMAP_TYPE_PNG);
	g_errorInd = new wxImage(dirPref + appSkin->GetErrorInd(), wxBITMAP_TYPE_PNG);
	
}

void ClientStateIndicator::CreateComponent(){
	//Set Background color
	SetBackgroundColour(appSkin->GetAppBgCol());
}
void ClientStateIndicator::SetStateConnectingToClient()
{
	Freeze();
	//Delete Previous state
	DeletePreviousState();

	clientCurrState = "connecting";
	i_indBg = new ImageLoader(this);
	i_indBg->Move(wxPoint(42,74));
	i_indBg->LoadImage(g_stateIndBg);

	stateMessage = wxString("CONNECTING TO CLIENT");
	
	for(int x = 0; x < numOfIndic; x++){
        ImageLoader *i_connInd = new ImageLoader(this);
		i_connInd->Move(wxPoint(rightPosition +(connIndicatorWidth+10) * x,84));
		i_connInd->LoadImage(g_connInd);
		if(x !=0){
            i_connInd->Show(false);
		}
		m_connIndV.push_back(i_connInd);
	}
	//set animation timer for interface
	m_connRenderTimer = new wxTimer(this, ID_ANIMATIONRENDERTIMER);
	wxASSERT(m_connRenderTimer);
    m_connRenderTimer->Start(400); 
    Thaw();
}
void ClientStateIndicator::SetNoWorkPresentState()
{
	Freeze();
	//Delete Previous state
	DeletePreviousState();

    clientCurrState = "nowork";
	i_indBg = new ImageLoader(this);
	i_indBg->Move(wxPoint(42,74));
	i_indBg->LoadImage(g_stateIndBg);
	stateMessage = wxString("NO WORK PRESENT"); 

	i_errorInd = new ImageLoader(this);
	i_errorInd->Move(wxPoint(rightPosition,84));
	i_errorInd->LoadImage(g_errorInd);
	i_errorInd->Refresh();
	Thaw();	
}
void ClientStateIndicator::DeletePreviousState()
{
	if(clientCurrState == "connecting"){
		if (m_connRenderTimer) {
			m_connRenderTimer->Stop();
			delete m_connRenderTimer;
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
	}else if(clientCurrState == "nowork"){
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
	LoadSkinImages();

	if(clientCurrState == "connecting"){
		SetStateConnectingToClient();
	}else if(clientCurrState == "nowork"){
		SetNoWorkPresentState();
	}

}
void ClientStateIndicator::OnPaint(wxPaintEvent& WXUNUSED(event)) 
{ 
    wxPaintDC dc(this);
	//static: message
	dc.SetFont(wxFont(9,74,90,90,0,wxT("Arial")));
	dc.DrawText(stateMessage, wxPoint(47,120)); 
     	
}
void ClientStateIndicator::OnEraseBackground(wxEraseEvent& event){

	event.Skip(false);
	wxDC *dc;
	dc=event.GetDC();
	dc->SetBackground(wxBrush(this->GetBackgroundColour(),wxSOLID));
	dc->Clear();
	if(m_compBG.Ok()) 
    { 
		dc->DrawBitmap(m_compBG, 0, 0); 
    } 
	
}
