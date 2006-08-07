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
	//app skin class
	appSkin = SkinClass::Instance();
	wxString dirPref = appSkin->GetSkinsFolder()+_T("/")+appSkin->GetSkinName()+_T("/");
	//bg
    g_gaugeBg = new wxImage(dirPref + wxT("graphic/gauge_bg.png"), wxBITMAP_TYPE_PNG);
	m_gaugeBG = wxBitmap(g_gaugeBg); 
	// indicator
	g_gaugeInd = new wxImage(dirPref + wxT("graphic/gauge_progress_indicator.png"), wxBITMAP_TYPE_PNG);
	

	
}
void CProgressBar::SetValue(double progress)
{
	numOfProgressInd = progress /(100/numOfIndic);
	
	for(int indIndex = 0; indIndex < numOfProgressInd; indIndex++){
		wxWindow *w_ind=new wxWindow(this,-1,wxPoint(rightPosition +((indicatorWidth+1)*indIndex),topPosition),wxSize(indicatorWidth,indicatorHeight));
        ImageLoader *i_ind = new ImageLoader(w_ind);
		i_ind->LoadImage(g_gaugeInd);
	}


}
void CProgressBar::UpdateValue(double progress)
{
	int currProg = progress /(100/numOfIndic);
	if(numOfProgressInd < currProg){
		wxWindow *w_ind=new wxWindow(this,-1,wxPoint(rightPosition +((indicatorWidth+1)*numOfProgressInd),topPosition),wxSize(indicatorWidth,indicatorHeight));
        ImageLoader *i_ind = new ImageLoader(w_ind);
		i_ind->LoadImage(g_gaugeInd);
		numOfProgressInd = progress /(100/numOfIndic);
	}
}
void CProgressBar::OnEraseBackground(wxEraseEvent& event){

	event.Skip(false);
	wxDC *dc;
	dc=event.GetDC();
	dc->SetBackground(wxBrush(this->GetBackgroundColour(),wxSOLID));
	dc->Clear();
	if(m_gaugeBG.Ok()) 
    { 
		dc->DrawBitmap(m_gaugeBG, 0, 0); 
    } 
	
}
