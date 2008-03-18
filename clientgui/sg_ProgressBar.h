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

#ifndef _PROGRESSBAR_H_
#define _PROGRESSBAR_H_ 

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "sg_ProgressBar.cpp"
#endif

class ImageLoader;

class CProgressBar : public wxPanel
{ 
public: 
		int indicatorWidth;
		int indicatorHeight;
		int numOfIndic;
		int rightPosition;
	    int topPosition;
		std::vector<ImageLoader*> m_progInd;
	    /// Constructors
	    CProgressBar(wxPanel* parent, wxPoint coord); 
		void SetValue(double progress);
		void ReskinInterface();
		void LoadIndicators();
private: 
        
		double m_progress;
		int m_numOfProgressInd;

		void OnEraseBackground(wxEraseEvent& event);

        DECLARE_EVENT_TABLE() 
}; 

#endif 

