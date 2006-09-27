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
	m_numOfProgressInd = 0;
	appSkin = SkinClass::Instance();
	LoadIndicators();
}

void CProgressBar::LoadIndicators() {
	// Remove any currently loaded
	int indIndex = 0;
	int x_pos;
    wxLogTrace(wxT("Function Start/End"), wxT("CProgressBar::LoadIndicators - Function Start"));
	for(indIndex = (int) m_progInd.size()-1; indIndex >= 0; indIndex--){
		delete m_progInd.at(indIndex);
	}
    m_progInd.clear();

	// Load all new ones but do not display
	for(indIndex=0; indIndex < numOfIndic; indIndex++) {
		ImageLoader *i_ind = new ImageLoader(this);
		x_pos = rightPosition +((indicatorWidth)*indIndex);
        i_ind->Move(wxPoint(x_pos,topPosition));
		i_ind->LoadImage(*(appSkin->GetGaugeProgressInd()));
		i_ind->Show(true);
		m_progInd.push_back(i_ind);
	}

    wxLogTrace(wxT("Function Start/End"), wxT("CProgressBar::LoadIndicators - Function End"));
}
void CProgressBar::SetValue(double progress)
{
	int numOfProgressInd = progress /(100/numOfIndic);
	int indIndex = 0;
	
	for(indIndex = 0; indIndex < numOfIndic; indIndex++){
		ImageLoader *i_ind = m_progInd.at(indIndex);
		if ( indIndex + 1 <= numOfProgressInd ) {
			i_ind->Show(true);
		} else {
			i_ind->Show(false);
		}
	}

	m_progress = progress;
	m_numOfProgressInd = numOfProgressInd;
}
void CProgressBar::UpdateValue(double progress)
{	
	int indIndex = 0;
	int numOfProgressInd = progress /(100/numOfIndic);

	if ( numOfProgressInd > m_numOfProgressInd ) {
		for(indIndex=m_numOfProgressInd; indIndex < numOfProgressInd; indIndex++) {
			ImageLoader *i_ind = m_progInd.at(indIndex);
			i_ind->Show(true);
		}
	} else if ( numOfProgressInd < m_numOfProgressInd ) {
		for(indIndex=m_numOfProgressInd - 1; indIndex > numOfProgressInd - 1; indIndex--) {
			ImageLoader *i_ind = m_progInd.at(indIndex);
			i_ind->Show(false);
		}
	}

	m_progress = progress;
	m_numOfProgressInd = numOfProgressInd;
}
void CProgressBar::ReskinInterface()
{
	LoadIndicators();
	SetValue(m_progress);
}
void CProgressBar::OnEraseBackground(wxEraseEvent& event){

	event.Skip(false);
	wxDC *dc;
	dc=event.GetDC();
	dc->SetBackground(wxBrush(this->GetBackgroundColour(),wxSOLID));
	dc->Clear();
	dc->DrawBitmap(*(appSkin->GetGaugeBg()), 0, 0); 
}
