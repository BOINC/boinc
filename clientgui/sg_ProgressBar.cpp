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
#pragma implementation "sg_ProgressBar.h"
#endif

#include "stdwx.h"
#include "sg_SkinClass.h"
#include "sg_ImageLoader.h"
#include "sg_ProgressBar.h" 

BEGIN_EVENT_TABLE(CProgressBar, wxWindow) 
		EVT_ERASE_BACKGROUND(CProgressBar::OnEraseBackground)
END_EVENT_TABLE() 

CProgressBar::CProgressBar(wxPanel* parent,wxPoint coord) : wxPanel(parent, wxID_ANY, coord, wxSize(258,18), wxNO_BORDER) 
{
	indicatorWidth = 8;
	indicatorHeight = 7;
	numOfIndic = 30;
	rightPosition = 9;
	topPosition = 5;
	m_progress = 0;
	numOfProgressInd = 0;
	appSkin = SkinClass::Instance();
}
void CProgressBar::SetValue(double progress)
{
	// clrear prior indicators
	ClearIndicators();

	m_progress = progress;
	numOfProgressInd = progress /(100/numOfIndic);
	
	for(int indIndex = 0; indIndex < numOfProgressInd; indIndex++){
		ImageLoader *i_ind = new ImageLoader(this);
        i_ind->Move(wxPoint(rightPosition +((indicatorWidth)*indIndex),topPosition));
		i_ind->LoadImage(*(appSkin->GetGaugeProgressInd()));
		m_progInd.push_back(i_ind);
	}
}
void CProgressBar::UpdateValue(double progress)
{	
	// if updating with smaller value clear indicators first
	if(m_progress>progress){
		ClearIndicators();
	}
    m_progress = progress;
	int currProg = progress /(100/numOfIndic);
	if(numOfProgressInd < currProg){
		for(int indIndex = numOfProgressInd; indIndex < currProg; indIndex++){
			wxWindow *w_ind=new wxWindow(this,-1,wxPoint(rightPosition +((indicatorWidth)*indIndex),topPosition),wxSize(indicatorWidth,indicatorHeight));
			ImageLoader *i_ind = new ImageLoader(w_ind);
			i_ind->LoadImage(*(appSkin->GetGaugeProgressInd()));
			numOfProgressInd = progress /(100/numOfIndic);
		}
	}
}
void CProgressBar::ReskinInterface()
{
	SetValue(m_progress);
}
void CProgressBar::ClearIndicators()
{
    wxLogTrace(wxT("Function Start/End"), wxT("CProgressBar::ClearIndicators - Function Start"));
	for(int indIndex = 0; indIndex < numOfProgressInd; indIndex++){
		delete m_progInd.at(indIndex);
	}
	//clear vector
	if(m_progInd.size() > 0){
        m_progInd.clear();
	}
    wxLogTrace(wxT("Function Start/End"), wxT("CProgressBar::ClearIndicators - Function End"));
}
void CProgressBar::OnEraseBackground(wxEraseEvent& event){

	event.Skip(false);
	wxDC *dc;
	dc=event.GetDC();
	dc->SetBackground(wxBrush(this->GetBackgroundColour(),wxSOLID));
	dc->Clear();
	dc->DrawBitmap(*(appSkin->GetGaugeBg()), 0, 0); 
}
