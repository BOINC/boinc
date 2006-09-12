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

	wxString dirPref = compute_skin_dir()+_T("/");

	if ( skinImageArray->Count() != 35 ) {
		return false;
	}
	
	for(unsigned int x = 0; x < skinImageArray->Count();x++){
		wxString imgLoc = skinImageArray->Item(x);
		wxBitmap skinImage = wxBitmap(dirPref + skinImageArray->Item(x),wxBITMAP_TYPE_PNG);
		if(!skinImage.Ok()){
			return false;
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
	// init skin image array
	skinImageArray = new wxArrayString();

    while (mf.fgets(buf, 256)) {
        if (match_tag(buf, "<clientskin")) {
            continue;
		}else if (match_tag(buf, "<simple")) {
            continue;
		}else if (parse_str(buf, "<background",val)) {
			SetAppBg(wxString( val.c_str(), wxConvUTF8 ));
			skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
			parse_attr(buf,"background_color", val1, buf_size);
			SetAppBgCol(wxString( val1, wxConvUTF8 ));
        }else if (parse_str(buf, "<project_component_background",val)) {
			SetProjCompBg(wxString( val.c_str(), wxConvUTF8 ));
			skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
        }else if (parse_str(buf, "<tab_area_background",val)) {
			SetTabAreaBg(wxString( val.c_str(), wxConvUTF8 ));
			skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
        }else if (parse_str(buf, "<spacer_image",val)) {
			SetSpacerImage(wxString( val.c_str(), wxConvUTF8 ));
			skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
        }else if (parse_str(buf, "<workunit_background",val)) {
			SetWorkunitBg(wxString( val.c_str(), wxConvUTF8 ));
			skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
        }else if (parse_str(buf, "<preferences_dialogue",val)) {
			SetDlgPrefBg(wxString( val.c_str(), wxConvUTF8 ));
			skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
        }else if (parse_str(buf, "<messages_dialogue",val)) {
			SetDlgMessBg(wxString( val.c_str(), wxConvUTF8 ));
			skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
        }else if (parse_str(buf, "<static_line_color",val)) {
			SetStaticLineCol(wxString( val.c_str(), wxConvUTF8 ));
        }else if (parse_str(buf, "<gauge_background",val)) {
			SetGaugeBg(wxString( val.c_str(), wxConvUTF8 ));
			skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
        }else if (parse_str(buf, "<gauge_progress",val)) {
			SetGaugeProgressInd(wxString( val.c_str(), wxConvUTF8 ));
			skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
        }else if (parse_str(buf, "<state_indicator_background",val)) {
			SetStateIndBg(wxString( val.c_str(), wxConvUTF8 ));
			skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
        }else if (parse_str(buf, "<connecting_indicator",val)) {
			SetConnInd(wxString( val.c_str(), wxConvUTF8 ));
			skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
        }else if (parse_str(buf, "<error_indicator",val)) {
			SetErrorInd(wxString( val.c_str(), wxConvUTF8 ));
			skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
		} else if (parse_str(buf, "<preferences_button", val)){
			SetBtnPrefer(wxString( val.c_str(), wxConvUTF8 ));
			skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
		} else if (parse_str(buf, "<add_project_button", val)){
			SetBtnAddProj(wxString( val.c_str(), wxConvUTF8 ));
			skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
		} else if (parse_str(buf, "<add_project_clicked_button", val)){
			SetBtnAddProjClick(wxString( val.c_str(), wxConvUTF8 ));
			skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
		} else if (parse_str(buf, "<advanced_view_button", val)){
			SetBtnAdvView(wxString( val.c_str(), wxConvUTF8 ));
			skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
		} else if (parse_str(buf, "<resume_button", val)){
			SetBtnResume(wxString( val.c_str(), wxConvUTF8 ));
			skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
		} else if (parse_str(buf, "<pause_button", val)){
			SetBtnPause(wxString( val.c_str(), wxConvUTF8 ));
			skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
		} else if (parse_str(buf, "<messages_button", val)){
			SetBtnMessages(wxString( val.c_str(), wxConvUTF8 ));
			skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
		} else if (parse_str(buf, "<alert_messages_button", val)){
			SetBtnAlertMessages(wxString( val.c_str(), wxConvUTF8 ));
			skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
		} else if (parse_str(buf, "<save_button", val)){
			SetBtnSave(wxString( val.c_str(), wxConvUTF8 ));
			skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
		} else if (parse_str(buf, "<save_clicked_button", val)){
			SetBtnSaveClick(wxString( val.c_str(), wxConvUTF8 ));
			skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
		} else if (parse_str(buf, "<cancel_button", val)){
			SetBtnCancel(wxString( val.c_str(), wxConvUTF8 ));
			skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
		} else if (parse_str(buf, "<cancel_clicked_button", val)){
			SetBtnCancelClick(wxString( val.c_str(), wxConvUTF8 ));
			skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
		} else if (parse_str(buf, "<close_button", val)){
			SetBtnClose(wxString( val.c_str(), wxConvUTF8 ));
			skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
		} else if (parse_str(buf, "<close_clicked_button", val)){
			SetBtnCloseClick(wxString( val.c_str(), wxConvUTF8 ));
			skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
		} else if (parse_str(buf, "<clear_button", val)){
			SetBtnClear(wxString( val.c_str(), wxConvUTF8 ));
			skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
		} else if (parse_str(buf, "<clear_clicked_button", val)){
			SetBtnClearClick(wxString( val.c_str(), wxConvUTF8 ));
			skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
		} else if (parse_str(buf, "<left_arrow_button", val)){
			SetBtnLeftArr(wxString( val.c_str(), wxConvUTF8 ));
			skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
		} else if (parse_str(buf, "<left_arrow_clicked_button", val)){
			SetBtnLeftArrClick(wxString( val.c_str(), wxConvUTF8 ));
			skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
		} else if (parse_str(buf, "<right_arrow_button", val)){
			SetBtnRightArr(wxString( val.c_str(), wxConvUTF8 ));
			skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
		} else if (parse_str(buf, "<right_arrow_clicked_button", val)){
			SetBtnRightArrClick(wxString( val.c_str(), wxConvUTF8 ));
			skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
		} else if (parse_str(buf, "<animation_background", val)){
			SetAnimationBg(wxString( val.c_str(), wxConvUTF8 ));
			skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
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
					if (parse_str(buf, "<imgsrc>", val)) {
						SetIcnWorkingWkUnit(wxString( val.c_str(), wxConvUTF8 ));
						skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
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
					if (parse_str(buf, "<imgsrc>", val)) {
						SetDefaultStatIcn(wxString( val.c_str(), wxConvUTF8 ));
						skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
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
		return false;
	}
}

