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
#include "error_numbers.h"
#include "miofile.h"
#include "parse.h"

SkinClass::SkinClass() 
{ 
	m_skinsFolder = "skins";// skins is the default folder
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

bool SkinClass::CheckSkin()
{
	//load skin xml file first
	if(!LoadSkinXML()==0){
		return false;//skin xml file is not available
	}

	if ( skinImageNames.size() != 35 ) {
		return false;
	}
	
	return LoadImages();;
}

bool SkinClass::LoadImages() {
	wxString dirPref = compute_skin_dir()+_T("/");
	for( wxStringHashMap::iterator x = skinImageNames.begin(); x != skinImageNames.end();x++){
		wxString imgLoc = x->second;
		wxBitmap* skinImage = new wxBitmap(dirPref + imgLoc,wxBITMAP_TYPE_ANY);
		if(!skinImage->Ok()){
			return false;
		} else {
			wxBitmapHashMap::iterator y = skinImages.find(x->first);
			if ( y != skinImages.end() ) {
				delete y->second;
				skinImages.erase(y);
			}
			skinImages[x->first] = skinImage;
		}
	}
     
	return true;
}

wxString SkinClass::compute_skin_dir() {
	return GetSkinsFolder()+_T("/")+GetSkinName();
}

wxString SkinClass::compute_skin_path() {
	return compute_skin_dir()+_T("/")+_T("skin.xml");
}

bool SkinClass::GetImageName(char* buf, const char* field) {
    std::string val;
	std::string start = "<";
	if (parse_str(buf, start.append(field).c_str(),val)) {
		skinImageNames[wxString(_T(field))]=wxString( val.c_str(), wxConvUTF8 );
		return true;
	} else {
		return false;
	}
}

int SkinClass::LoadSkinXML(){
   
	// parse xml file		    
	FILE* f;
    f = fopen(compute_skin_path(), "r");
	if (!f) return ERR_FOPEN;
    MIOFILE mf;
    mf.init_file(f);
    // parse
	int buf_size = 256;
	char buf[256];
    std::string val;
	char val1[256];

    while (mf.fgets(buf, 256)) {
        if (match_tag(buf, "<clientskin")) {
            continue;
		} else if (match_tag(buf, "<simple")) {
            continue;
		} else if (GetImageName(buf, "background")) {
			parse_attr(buf,"background_color", val1, buf_size);
			SetAppBgCol(wxString( val1, wxConvUTF8 ));
        } else if (GetImageName(buf, "project_component_background")) {
        } else if (GetImageName(buf, "tab_area_background")) {
        } else if (GetImageName(buf, "spacer_image")) {
        } else if (GetImageName(buf, "workunit_background")) {
        } else if (GetImageName(buf, "preferences_dialogue")) {
        } else if (GetImageName(buf, "messages_dialogue")) {
        } else if (parse_str(buf, "<static_line_color",val)) {
			SetStaticLineCol(wxString( val.c_str(), wxConvUTF8 ));
        } else if (GetImageName(buf, "gauge_background")) {
        } else if (GetImageName(buf, "gauge_progress")) {
        } else if (GetImageName(buf, "state_indicator_background")) {
        } else if (GetImageName(buf, "connecting_indicator")) {
        } else if (GetImageName(buf, "error_indicator")) {
		} else if (GetImageName(buf, "preferences_button")){
		} else if (GetImageName(buf, "add_project_button")){
		} else if (GetImageName(buf, "add_project_clicked_button")){
		} else if (GetImageName(buf, "advanced_view_button")){
		} else if (GetImageName(buf, "resume_button")){
		} else if (GetImageName(buf, "pause_button")){
		} else if (GetImageName(buf, "messages_button")){
		} else if (GetImageName(buf, "alert_messages_button")){
		} else if (GetImageName(buf, "save_button")){
		} else if (GetImageName(buf, "save_clicked_button")){
		} else if (GetImageName(buf, "cancel_button")){
		} else if (GetImageName(buf, "cancel_clicked_button")){
		} else if (GetImageName(buf, "close_button")){
		} else if (GetImageName(buf, "close_clicked_button")){
		} else if (GetImageName(buf, "clear_button")){
		} else if (GetImageName(buf, "clear_clicked_button")){
		} else if (GetImageName(buf, "left_arrow_button")){
		} else if (GetImageName(buf, "left_arrow_clicked_button")){
		} else if (GetImageName(buf, "right_arrow_button")){
		} else if (GetImageName(buf, "right_arrow_clicked_button")){
		} else if (GetImageName(buf, "animation_background")){
		} else if (parse_str(buf, "<default_animation", val)){
			SetAnimationFile(wxString( val.c_str(), wxConvUTF8 ));
		} else if (match_tag(buf, "<icons")) {
			while (mf.fgets(buf, 256)) {
				std::string val;
				if(match_tag(buf, "</icons>")){
					//end of the buttons elements break out of while loop
					break;
				}else if(match_tag(buf, "<workingWkUnit>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrc", val)) {
						skinImageNames[wxString(_T("workingWkUnit"))]=wxString( val.c_str(), wxConvUTF8 );
					}
					mf.fgets(buf, 256);
					if (parse_str(buf, "<frcol>", val)) {
						SetTabFromColAc(wxString( val.c_str(), wxConvUTF8 ));
					}
					mf.fgets(buf, 256);
					if (parse_str(buf, "<tocol>", val)) {
						SetTabToColAc(wxString( val.c_str(), wxConvUTF8 ));
					}
					mf.fgets(buf, 256);
					if (parse_str(buf, "<brdcol>", val)) {
						SetTabBrdColAc(wxString( val.c_str(), wxConvUTF8 ));
					}
				}else if(match_tag(buf, "<sleepingWkUnit>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<frcol>", val)) {
						SetTabFromColIn(wxString( val.c_str(), wxConvUTF8 ));
					}
					mf.fgets(buf, 256);
					if (parse_str(buf, "<tocol>", val)) {
						SetTabToColIn(wxString( val.c_str(), wxConvUTF8 ));
					}
					mf.fgets(buf, 256);
					if (parse_str(buf, "<brdcol>", val)) {
						SetTabBrdColIn(wxString( val.c_str(), wxConvUTF8 ));
					}
				}else if(match_tag(buf, "<defaultStatIcon>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrc", val)) {
						skinImageNames[wxString(_T("defaultStatIcon"))]=wxString( val.c_str(), wxConvUTF8 );
					}
				}
			}// end of while loop
		}
	}
	//
    fclose(f);
	return 0;
}

bool SkinClass::change_skin(const wxString& new_skin_name) {
	wxString old_skin_name = m_skinName;
	// set new skin name
	SetSkinName(new_skin_name);
	if ( CheckSkin() ) {
		return true;
	} else {
		m_skinName = old_skin_name;
		LoadSkinXML();
		LoadImages();
		return false;
	}
}

bool SkinClass::init_skin(const wxString& skin_name) {
	SetSkinName(skin_name);
	if ( CheckSkin() ) {
		return true;
	} else {
		m_skinName = wxString("default");
		LoadSkinXML();
		LoadImages();
		return false;
	}
}

