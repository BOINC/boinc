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

#ifndef _STATIMAGELOADER_H_
#define _STATIMAGELOADER_H_ 

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "sg_StatImageLoader.cpp"
#endif


class StatImageLoader : public wxWindow 
{ 
public: 
    wxMenu *statPopUpMenu;
	char project_url[256];
	StatImageLoader(wxWindow* parent, char* url); 
	~StatImageLoader(); 
	void LoadImage();
	void OnMenuLinkClicked(wxCommandEvent& event);
	void OnProjectDetach();
	void PopUpMenu(wxMouseEvent& event); 
    void OnPaint(wxPaintEvent& event);
	void RebuildMenu();
	void UpdateInterface();

private: 

    wxBitmap Bitmap; 
	std::string projectIcon;
	int numReloadTries;
	size_t urlCount;
	double project_files_downloaded_time;
	double project_last_rpc_time;
	void LoadStatIcon(wxBitmap& image);
	void ReloadProjectSpecificIcon();
	void BuildUserStatToolTip();
	void AddMenuItems();
	std::string GetProjectIconLoc();
    DECLARE_EVENT_TABLE() 
}; 

#endif 

