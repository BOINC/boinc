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
#pragma implementation "sg_SkinClass.h"
#endif

#include "stdwx.h"
#include "sg_SkinClass.h" 
#include <string>

SkinClass::SkinClass() 
{ 
}
SkinClass* SkinClass::Instance() 
{ 
	static SkinClass inst;
    return &inst;
}
wxColour SkinClass::GetColorFromStr(wxString col){
	// create color
	int delPos = col.Find(":");
	wxString rcol = col.Mid(0,delPos);
	col = col.Mid(delPos+1, col.Length() - delPos);
	delPos = col.Find(":");
    wxString gcol = col.Mid(0,delPos);
	wxString bcol = col.Mid(delPos+1, col.Length()- delPos);

	unsigned char r_ch = atoi(rcol.c_str());
	unsigned char g_ch = atoi(gcol.c_str());
	unsigned char b_ch = atoi(bcol.c_str());

    return wxColour(r_ch,g_ch,b_ch);
}
