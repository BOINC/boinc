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
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef _CLIENTSTATEINDICATOR_H_
#define _CLIENTSTATEINDICATOR_H_ 

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "sg_ClientStateIndicator.cpp"
#endif


class ImageLoader;

class ClientStateIndicator : public wxPanel{ 
	DECLARE_DYNAMIC_CLASS( ClientStateIndicator )
public: 
		int connIndicatorWidth;
		int connIndicatorHeight;
		int numOfIndic;
		int rightPosition;
	    int topPosition;
		int indexIndVis;
		int numOfProgressInd;
		ImageLoader *i_indBg;
		ImageLoader *i_errorInd;
		std::vector<ImageLoader*> m_connIndV;
		wxTimer *m_connRenderTimer;
		wxString stateMessage;
		int clientState;
	    /// Constructors
		ClientStateIndicator();
	    ClientStateIndicator(CSimplePanel* parent, wxPoint coord);
		~ClientStateIndicator();
		void CreateComponent();
		void ReskinInterface();
		void DeletePreviousState();
		void DisplayState();

private: 
        
		void SetActionState(wxString message);
		void SetNoActionState(wxString message);
		void SetPausedState(wxString message);
		bool DownloadingResults();
		bool Suspended();
		bool ProjectUpdateScheduled();
		time_t error_time;
		
		void OnEraseBackground(wxEraseEvent& event);
		void OnPaint(wxPaintEvent& event); 
		void RunConnectionAnimation(wxTimerEvent& event );

        DECLARE_EVENT_TABLE() 
}; 

#define CLIENT_STATE_NONE 0
#define CLIENT_STATE_ACTION 1
#define CLIENT_STATE_PAUSED 2
#define CLIENT_STATE_ERROR 3

#endif 

