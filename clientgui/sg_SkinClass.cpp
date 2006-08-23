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

	wxString dirPref = GetSkinsFolder()+_T("/")+GetSkinName()+_T("/");
	
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
	char buf[256];
    std::string val;
	// init skin image array
	skinImageArray = new wxArrayString();

    while (mf.fgets(buf, 256)) {
        if (match_tag(buf, "<clientskin")) {
            continue;
		}else if (match_tag(buf, "<simple")) {
            continue;
		}else if (match_tag(buf, "<background")) {
			mf.fgets(buf, 256);
			if (parse_str(buf, "<imgsrc>", val)) {
				SetAppBg(wxString( val.c_str(), wxConvUTF8 ));
				skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
			}
			mf.fgets(buf, 256);
            if (parse_str(buf, "<bgcol>", val)) {
				SetAppBgCol(wxString( val.c_str(), wxConvUTF8 ));
			}
        }else if (match_tag(buf, "<prjcomponentbg")) {
			mf.fgets(buf, 256);
			if (parse_str(buf, "<imgsrc>", val)) {
				SetProjCompBg(wxString( val.c_str(), wxConvUTF8 ));
				skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
			}
        }else if (match_tag(buf, "<tabareabg")) {
			mf.fgets(buf, 256);
			if (parse_str(buf, "<imgsrc>", val)) {
				SetTabAreaBg(wxString( val.c_str(), wxConvUTF8 ));
				skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
			}
        }else if (match_tag(buf, "<spacerimage")) {
			mf.fgets(buf, 256);
			if (parse_str(buf, "<imgsrc>", val)) {
				SetSpacerImage(wxString( val.c_str(), wxConvUTF8 ));
				skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
			}
        }else if (match_tag(buf, "<workunitbg")) {
			mf.fgets(buf, 256);
			if (parse_str(buf, "<imgsrc>", val)) {
				SetWorkunitBg(wxString( val.c_str(), wxConvUTF8 ));
				skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
			}
        }else if (match_tag(buf, "<dlgpreferences")) {
			mf.fgets(buf, 256);
			if (parse_str(buf, "<imgsrc>", val)) {
				SetDlgPrefBg(wxString( val.c_str(), wxConvUTF8 ));
				skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
			}
        }else if (match_tag(buf, "<dlgmessages")) {
			mf.fgets(buf, 256);
			if (parse_str(buf, "<imgsrc>", val)) {
				SetDlgMessBg(wxString( val.c_str(), wxConvUTF8 ));
				skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
			}
        }else if (match_tag(buf, "<staticline")) {
			mf.fgets(buf, 256);
			if (parse_str(buf, "<col>", val)) {
				SetStaticLineCol(wxString( val.c_str(), wxConvUTF8 ));
			}
        }else if (match_tag(buf, "<gauge")) {
			mf.fgets(buf, 256);
			if (parse_str(buf, "<gaugebg>", val)) {
				SetGaugeBg(wxString( val.c_str(), wxConvUTF8 ));
				skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
			}
			mf.fgets(buf, 256);
            if (parse_str(buf, "<gaugeprogress>", val)) {
				SetGaugeProgressInd(wxString( val.c_str(), wxConvUTF8 ));
				skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
			}
        }else if (match_tag(buf, "<stateindicator")) {
			mf.fgets(buf, 256);
			if (parse_str(buf, "<stateindbg>", val)) {
				SetStateIndBg(wxString( val.c_str(), wxConvUTF8 ));
				skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
			}
			mf.fgets(buf, 256);
            if (parse_str(buf, "<connindicator>", val)) {
				SetConnInd(wxString( val.c_str(), wxConvUTF8 ));
				skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
			}
			mf.fgets(buf, 256);
            if (parse_str(buf, "<errorimage>", val)) {
				SetErrorInd(wxString( val.c_str(), wxConvUTF8 ));
				skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
			}
        }else if (match_tag(buf, "<buttons")) {
			while (mf.fgets(buf, 256)) {
				std::string val;
				if(match_tag(buf, "</buttons>")){
					//end of the buttons elements break out of while loop
					break;
				}
				if(match_tag(buf, "<preferences>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrc>", val)) {
						SetBtnPrefer(wxString( val.c_str(), wxConvUTF8 ));
						skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
					}
				}else if(match_tag(buf, "<addproj>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrc>", val)) {
						SetBtnAddProj(wxString( val.c_str(), wxConvUTF8 ));
						skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
					}
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrcclick>", val)) {
						SetBtnAddProjClick(wxString( val.c_str(), wxConvUTF8 ));
						skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
					}
				}else if(match_tag(buf, "<advancedview>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrc>", val)) {
						SetBtnAdvView(wxString( val.c_str(), wxConvUTF8 ));
						skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
					}
				}else if(match_tag(buf, "<resume>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrc>", val)) {
						SetBtnResume(wxString( val.c_str(), wxConvUTF8 ));
						skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
					}
				}else if(match_tag(buf, "<pause>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrc>", val)) {
						SetBtnPause(wxString( val.c_str(), wxConvUTF8 ));
						skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
					}
				}else if(match_tag(buf, "<messages>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrc>", val)) {
						SetBtnMessages(wxString( val.c_str(), wxConvUTF8 ));
						skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
					}
				}else if(match_tag(buf, "<alert_messages>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrc>", val)) {
						SetBtnAlertMessages(wxString( val.c_str(), wxConvUTF8 ));
						skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
					}
				}else if(match_tag(buf, "<save>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrc>", val)) {
						SetBtnSave(wxString( val.c_str(), wxConvUTF8 ));
						skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
					}
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrcclick>", val)) {
						SetBtnSaveClick(wxString( val.c_str(), wxConvUTF8 ));
						skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
					}
				}else if(match_tag(buf, "<cancel>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrc>", val)) {
						SetBtnCancel(wxString( val.c_str(), wxConvUTF8 ));
						skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
					}
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrcclick>", val)) {
						SetBtnCancelClick(wxString( val.c_str(), wxConvUTF8 ));
						skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
					}
				}else if(match_tag(buf, "<close>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrc>", val)) {
						SetBtnClose(wxString( val.c_str(), wxConvUTF8 ));
						skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
					}
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrcclick>", val)) {
						SetBtnCloseClick(wxString( val.c_str(), wxConvUTF8 ));
						skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
					}
				}else if(match_tag(buf, "<clear>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrc>", val)) {
						SetBtnClear(wxString( val.c_str(), wxConvUTF8 ));
						skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
					}
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrcclick>", val)) {
						SetBtnClearClick(wxString( val.c_str(), wxConvUTF8 ));
						skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
					}
				}else if(match_tag(buf, "<leftArr>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrc>", val)) {
						SetBtnLeftArr(wxString( val.c_str(), wxConvUTF8 ));
						skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
					}
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrcclick>", val)) {
						SetBtnLeftArrClick(wxString( val.c_str(), wxConvUTF8 ));
						skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
					}
				}else if(match_tag(buf, "<rightArr>")){
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrc>", val)) {
						SetBtnRightArr(wxString( val.c_str(), wxConvUTF8 ));
						skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
					}
					mf.fgets(buf, 256);
					if (parse_str(buf, "<imgsrcclick>", val)) {
						SetBtnRightArrClick(wxString( val.c_str(), wxConvUTF8 ));
						skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
					}
				}
			}//end of while
		}else if (match_tag(buf, "<icons")) {
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
		}else if (match_tag(buf, "<animation")) {
			mf.fgets(buf, 256);
			std::string val;
			if (parse_str(buf, "<background>", val)) {
				SetAnimationBg(wxString( val.c_str(), wxConvUTF8 ));
				skinImageArray->Add(wxString( val.c_str(), wxConvUTF8 ));
			}
			mf.fgets(buf, 256);
            if (parse_str(buf, "<animation>", val)) {
				SetAnimationFile(wxString( val.c_str(), wxConvUTF8 ));
			}
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

