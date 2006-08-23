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

#ifndef _CLIENTSTATEINDICATOR_H_
#define _CLIENTSTATEINDICATOR_H_ 

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "sg_ClientStateIndicator.cpp"
#endif

#include "time.h"

class ImageLoader;

class ClientStateIndicator : public wxPanel{ 
	DECLARE_DYNAMIC_CLASS( ClientStateIndicator )
public: 
	    //Skin Class
        SkinClass *appSkin;
		int connIndicatorWidth;
		int connIndicatorHeight;
		double numOfIndic;
		int rightPosition;
	    int topPosition;
		int indexIndVis;
		int numOfProgressInd;
		ImageLoader *i_indBg;
		ImageLoader *i_errorInd;
		std::vector<ImageLoader*> m_connIndV;
		wxTimer *m_connRenderTimer;
		wxString stateMessage;
		std::string clientCurrState;
	    /// Constructors
		ClientStateIndicator();
	    ClientStateIndicator(CSimpleFrame* parent, wxPoint coord);
		~ClientStateIndicator();
		void CreateComponent();
		void ReskinInterface();
		void LoadSkinImages();
		void DeletePreviousState();
		void DisplayState();

private: 
        
		void SetActionState(const char* message);
		void SetNoActionState(const char* message);
	    wxImage *g_stateIndBg;
		wxImage *g_compBg;
		wxImage *g_connInd;
		wxImage *g_errorInd;
        wxBitmap m_stateIndBG; 
		wxBitmap m_compBG;
		time_t error_time;
		
		void OnEraseBackground(wxEraseEvent& event);
		void OnPaint(wxPaintEvent& event); 
		void RunConnectionAnimation(wxTimerEvent& event );

        DECLARE_EVENT_TABLE() 
}; 

#endif 

