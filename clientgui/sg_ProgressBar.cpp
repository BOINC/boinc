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
#pragma implementation "sg_ProgressBar.h"
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
	LoadIndicators();
}

void CProgressBar::LoadIndicators() {
    CSkinSimple* pSkinSimple = wxGetApp().GetSkinManager()->GetSimple();
	int indIndex = 0;
    int indSize = 0;
	int x_pos;

    wxASSERT(pSkinSimple);
    wxASSERT(wxDynamicCast(pSkinSimple, CSkinSimple));

    wxLogTrace(wxT("Function Start/End"), wxT("CProgressBar::LoadIndicators - Function Start"));

	// Remove any currently loaded
    indSize = (int)m_progInd.size();
	for(indIndex = 0; indIndex < indSize; indIndex++){
		delete m_progInd[indIndex];
	}
    m_progInd.clear();

	// Load all new ones but do not display
	for(indIndex=0; indIndex < numOfIndic; indIndex++) {
		ImageLoader *i_ind = new ImageLoader(this);
		x_pos = rightPosition +((indicatorWidth)*indIndex);
        i_ind->Move(wxPoint(x_pos,topPosition));
        i_ind->LoadImage(*(pSkinSimple->GetWorkunitGaugeProgressIndicatorImage()->GetBitmap()));
		i_ind->Show(true);
		m_progInd.push_back(i_ind);
	}

    wxLogTrace(wxT("Function Start/End"), wxT("CProgressBar::LoadIndicators - Function End"));
}
void CProgressBar::SetValue(double progress)
{
	int indIndex = 0;
	int numOfProgressInd = ((int)progress/(100/numOfIndic));

    if (numOfProgressInd < 0) numOfProgressInd = 0;
    if (numOfProgressInd > numOfIndic) numOfProgressInd = numOfIndic;

	for(indIndex = 0; indIndex < numOfIndic; indIndex++){
		ImageLoader *i_ind = m_progInd[indIndex];
		if ( indIndex + 1 <= numOfProgressInd ) {
			i_ind->Show(true);
		} else {
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
    CSkinSimple* pSkinSimple = wxGetApp().GetSkinManager()->GetSimple();

    wxASSERT(pSkinSimple);
    wxASSERT(wxDynamicCast(pSkinSimple, CSkinSimple));

	event.Skip(false);
	wxDC *dc;
	dc=event.GetDC();
	dc->SetBackground(wxBrush(this->GetBackgroundColour(),wxSOLID));
	dc->Clear();
    dc->DrawBitmap(*(pSkinSimple->GetWorkunitGaugeBackgroundImage()->GetBitmap()), 0, 0); 
}
