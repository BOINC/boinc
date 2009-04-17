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
#pragma implementation "SkinManager.h"
#endif

#include "stdwx.h"
#include "diagnostics.h"
#include "parse.h"
#include "util.h"
#include "error_numbers.h"
#include "miofile.h"
#include "BOINCGUIApp.h"
#include "BOINCBaseFrame.h"
#include "SkinManager.h"
#include "version.h"


////@begin XPM images
#include "res/skins/default/graphic/background_image.xpm"
#include "res/skins/default/graphic/spacer_image.xpm"
#include "res/skins/default/graphic/state_indicator_background_image.xpm"
#include "res/skins/default/graphic/connecting_indicator_image.xpm"
#include "res/skins/default/graphic/error_indicator_image.xpm"
#include "res/skins/default/graphic/workunit_active_image.xpm"
#include "res/skins/default/graphic/workunit_suspended_image.xpm"
#include "res/skins/default/graphic/workunit_tab_area_background_image.xpm"
#include "res/skins/default/graphic/workunit_area_background_image.xpm"
#include "res/skins/default/graphic/workunit_animation_background_image.xpm"
#include "res/skins/default/graphic/workunit_animation_image.xpm"
#include "res/skins/default/graphic/workunit_gauge_background_image.xpm"
#include "res/skins/default/graphic/workunit_gauge_progress_indicator_image.xpm"
#include "res/skins/default/graphic/project_area_background_image.xpm"
#include "res/skins/default/graphic/project_image.xpm"
#include "res/skins/default/graphic/attach_project_button.xpm"
#include "res/skins/default/graphic/attach_project_clicked_button.xpm"
#include "res/skins/default/graphic/help_button.xpm"
#include "res/skins/default/graphic/help_clicked_button.xpm"
#include "res/skins/default/graphic/right_arrow_button.xpm"
#include "res/skins/default/graphic/right_arrow_clicked_button.xpm"
#include "res/skins/default/graphic/left_arrow_button.xpm"
#include "res/skins/default/graphic/left_arrow_clicked_button.xpm"
#include "res/skins/default/graphic/save_button.xpm"
#include "res/skins/default/graphic/save_clicked_button.xpm"
#include "res/skins/default/graphic/synchronize_button.xpm"
#include "res/skins/default/graphic/synchronize_clicked_button.xpm"
#include "res/skins/default/graphic/cancel_button.xpm"
#include "res/skins/default/graphic/cancel_clicked_button.xpm"
#include "res/skins/default/graphic/close_button.xpm"
#include "res/skins/default/graphic/close_clicked_button.xpm"
#include "res/skins/default/graphic/copy_all_button.xpm"
#include "res/skins/default/graphic/copy_all_clicked_button.xpm"
#include "res/skins/default/graphic/copy_button.xpm"
#include "res/skins/default/graphic/copy_clicked_button.xpm"
#include "res/skins/default/graphic/messages_link_image.xpm"
#include "res/skins/default/graphic/messages_alert_link_image.xpm"
#include "res/skins/default/graphic/suspend_link_image.xpm"
#include "res/skins/default/graphic/resume_link_image.xpm"
#include "res/skins/default/graphic/preferences_link_image.xpm"
#include "res/skins/default/graphic/advanced_link_image.xpm"
#include "res/skins/default/graphic/dialog_background_image.xpm"
#include "res/boinc.xpm"
#include "res/boinc32.xpm"
#include "res/boincdisconnect.xpm"
#include "res/boincsnooze.xpm"
#include "res/boinc_logo.xpm"
#include "res/wizard_bitmap.xpm"
////@end XPM images


// Flag to enable the various error messages
static bool show_error_msgs = false;


IMPLEMENT_DYNAMIC_CLASS(CSkinItem, wxObject)


CSkinItem::CSkinItem() {
}


CSkinItem::~CSkinItem() {
}


wxColour CSkinItem::ParseColor(wxString strColor) {
    long red, green, blue;
    wxStringTokenizer tkz(strColor, wxT(":"), wxTOKEN_RET_EMPTY);
    wxString(tkz.GetNextToken()).ToLong(&red);
	wxString(tkz.GetNextToken()).ToLong(&green);
	wxString(tkz.GetNextToken()).ToLong(&blue);
    return wxColour((unsigned char)red, (unsigned char)green, (unsigned char)blue);
}


IMPLEMENT_DYNAMIC_CLASS(CSkinImage, CSkinItem)


CSkinImage::CSkinImage() {
    Clear();
}


CSkinImage::~CSkinImage() {
    Clear();
}


void CSkinImage::Clear() {
    m_strDesiredBitmap.Clear();
    m_strDesiredBackgroundColor.Clear();
    m_bmpBitmap = wxNullBitmap;
    m_colBackgroundColor = wxNullColour;
}


int CSkinImage::Parse(MIOFILE& in) {
    char buf[256];
    std::string strBuffer;

    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</image>")) break;
        else if (parse_str(buf, "<imagesrc>", strBuffer)) {
            if (strBuffer.length()) {
                m_strDesiredBitmap = wxString(
                    wxGetApp().GetSkinManager()->ConstructSkinPath() +
                    wxString(strBuffer.c_str(), wxConvUTF8)
                );
            }
            continue;
        } else if (parse_str(buf, "<background_color>", strBuffer)) {
            if (strBuffer.length()) {
                m_strDesiredBackgroundColor = wxString(strBuffer.c_str(), wxConvUTF8);
            }
            continue;
        }
    }

    return BOINC_SUCCESS;
}


wxBitmap* CSkinImage::GetBitmap() {
    Validate();
    return &m_bmpBitmap; 
}


wxColour* CSkinImage::GetBackgroundColor() {
    Validate();
    return &m_colBackgroundColor;
}


bool CSkinImage::SetDefaults(wxString strComponentName, const char** ppDefaultBitmap) {
    m_strComponentName = strComponentName;
    m_ppDefaultBitmap = ppDefaultBitmap;
    return true;
}


bool CSkinImage::SetDefaults(wxString strComponentName, const char** ppDefaultBitmap, wxString strBackgroundColor) {
    m_strComponentName = strComponentName;
    m_ppDefaultBitmap = ppDefaultBitmap;
    m_strDefaultBackgroundColor = strBackgroundColor;
    return true;
}


bool CSkinImage::Validate() {
    if (!m_bmpBitmap.Ok()) {
        if (!m_strDesiredBitmap.IsEmpty()) {
            m_bmpBitmap = wxBitmap(wxImage(m_strDesiredBitmap, wxBITMAP_TYPE_ANY));
        }
        if (!m_bmpBitmap.Ok()) {
            if (show_error_msgs) {
                fprintf(stderr, "Skin Manager: Failed to load '%s' image. Using default.\n", (char *)m_strComponentName.c_str());
            }
            m_bmpBitmap = wxBitmap(m_ppDefaultBitmap);
            wxASSERT(m_bmpBitmap.Ok());
        }
    }
    if (!m_colBackgroundColor.Ok()) {
        if (!m_strDesiredBackgroundColor.IsEmpty()) {
            m_colBackgroundColor = ParseColor(m_strDesiredBackgroundColor);
        }
        if (!m_colBackgroundColor.Ok()) {
            if (show_error_msgs) {
                fprintf(stderr, "Skin Manager: Failed to load '%s' background color. Using default.\n", (char *)m_strComponentName.c_str());
            }
            m_colBackgroundColor = ParseColor(m_strDefaultBackgroundColor);
            wxASSERT(m_colBackgroundColor.Ok());
        }
    }
    return true;
}


IMPLEMENT_DYNAMIC_CLASS(CSkinIcon, CSkinItem)


CSkinIcon::CSkinIcon() {
    Clear();
}


CSkinIcon::~CSkinIcon() {
    Clear();
}


void CSkinIcon::Clear() {
    m_strDesiredIcon.Clear();
    m_strDesiredTransparencyMask.Clear();
    m_icoIcon = wxNullIcon;
}


int CSkinIcon::Parse(MIOFILE& in) {
    char buf[256];
    std::string strBuffer;

    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</image>")) break;
        else if (parse_str(buf, "<imagesrc>", strBuffer)) {
            if (strBuffer.length()) {
                m_strDesiredIcon = wxString(
                    wxGetApp().GetSkinManager()->ConstructSkinPath() +
                    wxString(strBuffer.c_str(), wxConvUTF8)
                );
            }
            continue;
        } else if (parse_str(buf, "<transparency_mask>", strBuffer)) {
            if (strBuffer.length()) {
                m_strDesiredTransparencyMask = wxString(strBuffer.c_str(), wxConvUTF8);
            }
            continue;
        }
    }

    return BOINC_SUCCESS;
}


wxIcon* CSkinIcon::GetIcon() {
    Validate();
    return &m_icoIcon;
}


bool CSkinIcon::SetDefaults(wxString strComponentName, const char** ppDefaultIcon) {
    m_strComponentName = strComponentName;
    m_ppDefaultIcon = ppDefaultIcon;
    return true;
}


bool CSkinIcon::Validate() {
    if (!m_icoIcon.Ok()) {
        if (!m_strDesiredIcon.IsEmpty()) {
            // Configure bitmap object with optional transparency mask
            wxBitmap bmp = wxBitmap(wxImage(m_strDesiredIcon, wxBITMAP_TYPE_ANY));
            if (!m_strDesiredTransparencyMask.IsEmpty()) {
                bmp.SetMask(new wxMask(bmp, ParseColor(m_strDesiredTransparencyMask)));
            }
            // Now set the icon object using the newly created bitmap with optional transparency mask
            m_icoIcon.CopyFromBitmap(bmp);
        }
        if (!m_icoIcon.Ok()) {
            if (show_error_msgs) {
                fprintf(stderr, "Skin Manager: Failed to load '%s' icon. Using default.\n", (char *)m_strComponentName.c_str());
            }
            m_icoIcon = wxIcon(m_ppDefaultIcon);
            wxASSERT(m_icoIcon.Ok());
        }
    }
    return true;
}


IMPLEMENT_DYNAMIC_CLASS(CSkinSimpleButton, CSkinItem)


CSkinSimpleButton::CSkinSimpleButton() {
    Clear();
}


CSkinSimpleButton::~CSkinSimpleButton() {
    Clear();
}


void CSkinSimpleButton::Clear() {
    m_strDesiredBitmap.Clear();
    m_strDesiredBitmapClicked.Clear();
    m_bmpBitmap = wxNullBitmap;
    m_bmpBitmapClicked = wxNullBitmap;
}


int CSkinSimpleButton::Parse(MIOFILE& in) {
    char buf[256];
    std::string strBuffer;

    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</button>")) break;
        else if (parse_str(buf, "<imagesrc>", strBuffer)) {
            if (strBuffer.length()) {
                m_strDesiredBitmap = wxString(
                    wxGetApp().GetSkinManager()->ConstructSkinPath() +
                    wxString(strBuffer.c_str(), wxConvUTF8)
                );
            }
            continue;
        }
        else if (parse_str(buf, "<imagesrc_clicked>", strBuffer)) {
            if (strBuffer.length()) {
                m_strDesiredBitmapClicked = wxString(
                    wxGetApp().GetSkinManager()->ConstructSkinPath() +
                    wxString(strBuffer.c_str(), wxConvUTF8)
                );
            }
            continue;
        }
    }

    return 0;
}


wxBitmap* CSkinSimpleButton::GetBitmap() {
    Validate();
    return &m_bmpBitmap;
}


wxBitmap* CSkinSimpleButton::GetBitmapClicked() {
    Validate();
    return &m_bmpBitmapClicked;
}


bool CSkinSimpleButton::SetDefaults(wxString strComponentName, const char** ppDefaultImage, const char** ppDefaultClickedImage) {
    m_strComponentName = strComponentName;
    m_ppDefaultBitmap = ppDefaultImage;
    m_ppDefaultBitmapClicked = ppDefaultClickedImage;
    return true;
}


bool CSkinSimpleButton::Validate() {
    if (!m_bmpBitmap.Ok()) {
        if (!m_strDesiredBitmap.IsEmpty()) {
            m_bmpBitmap = wxBitmap(wxImage(m_strDesiredBitmap, wxBITMAP_TYPE_ANY));
        }
        if (!m_bmpBitmap.Ok()) {
            if (show_error_msgs) {
                fprintf(stderr, "Skin Manager: Failed to load '%s' image. Using default.\n", (char *)m_strComponentName.c_str());
            }
            m_bmpBitmap = wxBitmap(m_ppDefaultBitmap);
            wxASSERT(m_bmpBitmap.Ok());
        }
    }
    if (!m_bmpBitmapClicked.Ok()) {
        if (!m_strDesiredBitmapClicked.IsEmpty()) {
            m_bmpBitmapClicked = wxBitmap(wxImage(m_strDesiredBitmapClicked, wxBITMAP_TYPE_ANY));
        }
        if (!m_bmpBitmapClicked.Ok()) {
            if (show_error_msgs) {
                fprintf(stderr, "Skin Manager: Failed to load '%s' clicked image. Using default.\n", (char *)m_strComponentName.c_str());
            }
            m_bmpBitmapClicked = wxBitmap(m_ppDefaultBitmapClicked);
            wxASSERT(m_bmpBitmapClicked.Ok());
        }
    }
    return true;
}


IMPLEMENT_DYNAMIC_CLASS(CSkinSimpleTab, CSkinItem)


CSkinSimpleTab::CSkinSimpleTab() {
    Clear();
}


CSkinSimpleTab::~CSkinSimpleTab() {
    Clear();
}


void CSkinSimpleTab::Clear() {
    m_strDesiredBitmap.Clear();
    m_strDesiredBorderColor.Clear();
    m_strDesiredGradientFromColor.Clear();
    m_strDesiredGradientToColor.Clear();
    m_bmpBitmap = wxNullBitmap;
    m_colBorderColor = wxNullColour;
    m_colGradientFromColor = wxNullColour;
    m_colGradientToColor = wxNullColour;
}


int CSkinSimpleTab::Parse(MIOFILE& in) {
    char buf[256];
    std::string strBuffer;

    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</tab>")) break;
        else if (parse_str(buf, "<imagesrc>", strBuffer)) {
            if (strBuffer.length()) {
                m_strDesiredBitmap = wxString(
                    wxGetApp().GetSkinManager()->ConstructSkinPath() +
                    wxString(strBuffer.c_str(), wxConvUTF8)
                );
            }
            continue;
        }
        else if (parse_str(buf, "<border_color>", strBuffer)) {
            if (strBuffer.length()) {
                m_strDesiredBorderColor = wxString(strBuffer.c_str(), wxConvUTF8);
            }
            continue;
        }
        else if (parse_str(buf, "<gradient_from_color>", strBuffer)) {
            if (strBuffer.length()) {
                m_strDesiredGradientFromColor = wxString(strBuffer.c_str(), wxConvUTF8);
            }
            continue;
        }
        else if (parse_str(buf, "<gradient_to_color>", strBuffer)) {
            if (strBuffer.length()) {
                m_strDesiredGradientToColor = wxString(strBuffer.c_str(), wxConvUTF8);
            }
            continue;
        }
    }

    return 0;
}


wxBitmap* CSkinSimpleTab::GetBitmap() {
    Validate();
    return &m_bmpBitmap;
}


wxColour* CSkinSimpleTab::GetBorderColor() {
    Validate();
    return &m_colBorderColor;
}


wxColour* CSkinSimpleTab::GetGradientFromColor() {
    Validate();
    return &m_colGradientFromColor;
}


wxColour* CSkinSimpleTab::GetGradientToColor() {
    Validate();
    return &m_colGradientToColor;
}

   
bool CSkinSimpleTab::SetDefaults(
    wxString strComponentName, const char** ppDefaultImage, wxString strBorderColor, wxString strGradientFromColor, wxString strGradientToColor
) {
    m_strComponentName = strComponentName;
    m_ppDefaultBitmap = ppDefaultImage;
    m_strDefaultBorderColor = strBorderColor;
    m_strDefaultGradientFromColor = strGradientFromColor;
    m_strDefaultGradientToColor = strGradientToColor;
    return true;
}


bool CSkinSimpleTab::Validate() {
    if (!m_bmpBitmap.Ok()) {
        if (!m_strDesiredBitmap.IsEmpty()) {
            m_bmpBitmap = wxBitmap(wxImage(m_strDesiredBitmap, wxBITMAP_TYPE_ANY));
        }
        if (!m_bmpBitmap.Ok()) {
            if (show_error_msgs) {
                fprintf(stderr, "Skin Manager: Failed to load '%s' tab image. Using default.\n", (char *)m_strComponentName.c_str());
            }
            m_bmpBitmap = wxBitmap(m_ppDefaultBitmap);
            wxASSERT(m_bmpBitmap.Ok());
        }
    }
    if (!m_colBorderColor.Ok()) {
        if (!m_strDesiredBorderColor.IsEmpty()) {
            m_colBorderColor = ParseColor(m_strDesiredBorderColor);
        }
        if (!m_colBorderColor.Ok()) {
            if (show_error_msgs) {
                fprintf(stderr, "Skin Manager: Failed to load '%s' tab border color. Using default.\n", (char *)m_strComponentName.c_str());
            }
            m_colBorderColor = ParseColor(m_strDefaultBorderColor);
            wxASSERT(m_colBorderColor.Ok());
        }
    }
    if (!m_colGradientFromColor.Ok()) {
        if (!m_strDesiredGradientFromColor.IsEmpty()) {
            m_colGradientFromColor = ParseColor(m_strDesiredGradientFromColor);
        }
        if (!m_colGradientFromColor.Ok()) {
            if (show_error_msgs) {
                fprintf(stderr, "Skin Manager: Failed to load '%s' tab gradient from color. Using default.\n", (char *)m_strComponentName.c_str());
            }
            m_colGradientFromColor = ParseColor(m_strDefaultGradientFromColor);
            wxASSERT(m_colGradientFromColor.Ok());
        }
    }
    if (!m_colGradientToColor.Ok()) {
        if (!m_strDesiredGradientToColor.IsEmpty()) {
            m_colGradientToColor = ParseColor(m_strDesiredGradientToColor);
        }
        if (!m_colGradientToColor.Ok()) {
            if (show_error_msgs) {
                fprintf(stderr, "Skin Manager: Failed to load '%s' tab gradient to color. Using default.\n", (char *)m_strComponentName.c_str());
            }
            m_colGradientToColor = ParseColor(m_strDefaultGradientToColor);
            wxASSERT(m_colGradientToColor.Ok());
        }
    }
    return true;
}


IMPLEMENT_DYNAMIC_CLASS(CSkinSimple, CSkinItem)


CSkinSimple::CSkinSimple() {
    Clear();
}


CSkinSimple::~CSkinSimple() {
    Clear();
}


void CSkinSimple::Clear() {
	m_BackgroundImage.Clear();
    m_SpacerImage.Clear();
	m_StaticLineColor = wxNullColour;

    m_StateIndicatorBackgroundImage.Clear();
    m_ConnectingIndicatorImage.Clear();
    m_ErrorIndicatorImage.Clear();

    m_WorkunitActiveTab.Clear();
    m_WorkunitSuspendedTab.Clear();
    m_WorkunitTabAreaBackgroundImage.Clear();
    m_WorkunitAreaBackgroundImage.Clear();
    m_WorkunitAnimationBackgroundImage.Clear();
    m_WorkunitAnimationImage.Clear();
    m_WorkunitGaugeBackgroundImage.Clear();
    m_WorkunitGaugeProgressIndicatorImage.Clear();

    m_ProjectAreaBackgroundImage.Clear();
    m_ProjectImage.Clear();

    m_AttachProjectButton.Clear();
    m_RightArrowButton.Clear();
    m_LeftArrowButton.Clear();
    m_SaveButton.Clear();
    m_SynchronizeButton.Clear();
    m_CancelButton.Clear();
    m_CloseButton.Clear();
    m_CopyAllButton.Clear();
    m_CopyButton.Clear();
    m_HelpButton.Clear();

    m_DialogBackgroundImage.Clear();

    m_MessagesLink.Clear();
    m_MessagesAlertLink.Clear();
    m_SuspendLink.Clear();
    m_ResumeLink.Clear();
    m_PreferencesLink.Clear();
    m_AdvancedLink.Clear();
}


int CSkinSimple::Parse(MIOFILE& in) {
    char buf[256];
    std::string strBuffer;

    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</simple>")) break;
        else if (match_tag(buf, "<background_image>")) {
            m_BackgroundImage.Parse(in);
            continue;
        } else if (match_tag(buf, "<spacer_image>")) {
            m_SpacerImage.Parse(in);
            continue;
        } else if (parse_str(buf, "<static_line_color>", strBuffer)) {
            m_StaticLineColor = ParseColor(wxString(strBuffer.c_str(), wxConvUTF8));
            continue;
        } else if (match_tag(buf, "<state_indicator_background_image>")) {
            m_StateIndicatorBackgroundImage.Parse(in);
            continue;
        } else if (match_tag(buf, "<connecting_indicator_image>")) {
            m_ConnectingIndicatorImage.Parse(in);
            continue;
        } else if (match_tag(buf, "<error_indicator_image>")) {
            m_ErrorIndicatorImage.Parse(in);
            continue;
        } else if (match_tag(buf, "<workunit_active_tab>")) {
            m_WorkunitActiveTab.Parse(in);
            continue;
        } else if (match_tag(buf, "<workunit_suspended_tab>")) {
            m_WorkunitSuspendedTab.Parse(in);
            continue;
        } else if (match_tag(buf, "<workunit_tab_area_background_image>")) {
            m_WorkunitTabAreaBackgroundImage.Parse(in);
            continue;
        } else if (match_tag(buf, "<workunit_area_background_image>")) {
            m_WorkunitAreaBackgroundImage.Parse(in);
            continue;
        } else if (match_tag(buf, "<workunit_animation_background_image>")) {
            m_WorkunitAnimationBackgroundImage.Parse(in);
            continue;
        } else if (match_tag(buf, "<workunit_animation_image>")) {
            m_WorkunitAnimationImage.Parse(in);
            continue;
        } else if (match_tag(buf, "<workunit_gauge_background_image>")) {
            m_WorkunitGaugeBackgroundImage.Parse(in);
            continue;
        } else if (match_tag(buf, "<workunit_gauge_progress_indicator_image>")) {
            m_WorkunitGaugeProgressIndicatorImage.Parse(in);
            continue;
        } else if (match_tag(buf, "<project_area_background_image>")) {
            m_ProjectAreaBackgroundImage.Parse(in);
            continue;
        } else if (match_tag(buf, "<project_image>")) {
            m_ProjectImage.Parse(in);
            continue;
        } else if (match_tag(buf, "<attach_project_button>")) {
            m_AttachProjectButton.Parse(in);
            continue;
        } else if (match_tag(buf, "<help_button>")) {
            m_HelpButton.Parse(in);
            continue;
        } else if (match_tag(buf, "<right_arrow_button>")) {
            m_RightArrowButton.Parse(in);
            continue;
        } else if (match_tag(buf, "<left_arrow_button>")) {
            m_LeftArrowButton.Parse(in);
            continue;
        } else if (match_tag(buf, "<save_button>")) {
            m_SaveButton.Parse(in);
            continue;
        } else if (match_tag(buf, "<synchronize_button>")) {
            m_SynchronizeButton.Parse(in);
            continue;
        } else if (match_tag(buf, "<cancel_button>")) {
            m_CancelButton.Parse(in);
            continue;
        } else if (match_tag(buf, "<close_button>")) {
            m_CloseButton.Parse(in);
            continue;
        } else if (match_tag(buf, "<copy_all_button>")) {
            m_CopyAllButton.Parse(in);
            continue;
        } else if (match_tag(buf, "<copy_button>")) {
            m_CopyButton.Parse(in);
            continue;
        } else if (match_tag(buf, "<messages_link_image>")) {
            m_MessagesLink.Parse(in);
            continue;
        } else if (match_tag(buf, "<messages_alert_link_image>")) {
            m_MessagesAlertLink.Parse(in);
            continue;
        } else if (match_tag(buf, "<suspend_link_image>")) {
            m_SuspendLink.Parse(in);
            continue;
        } else if (match_tag(buf, "<resume_link_image>")) {
            m_ResumeLink.Parse(in);
            continue;
        } else if (match_tag(buf, "<preferences_link_image>")) {
            m_PreferencesLink.Parse(in);
            continue;
        } else if (match_tag(buf, "<advanced_link_image>")) {
            m_AdvancedLink.Parse(in);
            continue;
        } else if (match_tag(buf, "<dialog_background_image>")) {
            m_DialogBackgroundImage.Parse(in);
            continue;
        }
    }

    InitializeDelayedValidation();

    return 0;
}


bool CSkinSimple::InitializeDelayedValidation() {
    m_BackgroundImage.SetDefaults(
        wxT("background"), (const char**)background_image_xpm, wxT("133:181:178")
    );
    m_SpacerImage.SetDefaults(wxT("spacer"), (const char**)spacer_image_xpm);
    if (!m_StaticLineColor.Ok()) {
        if (show_error_msgs) {
            fprintf(stderr, "Skin Manager: Failed to parse static line color. Using default.\n");
        }
        m_StaticLineColor = ParseColor(wxString(wxT("204:102:51")));
        wxASSERT(m_StaticLineColor.Ok());
    }
    m_StateIndicatorBackgroundImage.SetDefaults(
        wxT("state indicator background"), (const char**)state_indicator_background_image_xpm
    );
    m_ConnectingIndicatorImage.SetDefaults(
        wxT("connecting indicator"), (const char**)connecting_indicator_image_xpm
    );
    m_ErrorIndicatorImage.SetDefaults(
        wxT("error indicator"), (const char**)error_indicator_image_xpm
    );
    m_WorkunitActiveTab.SetDefaults(
        wxT("active"), (const char**)workunit_active_image_xpm, wxString(wxT("20:82:82")), wxString(wxT("134:179:176")), wxString(wxT("51:102:102"))
    );
    m_WorkunitSuspendedTab.SetDefaults(
        wxT("suspended"), (const char**)workunit_suspended_image_xpm, wxString(wxT("102:153:153")), wxString(wxT("134:179:176")), wxString(wxT("84:175:175"))
    );
    m_WorkunitTabAreaBackgroundImage.SetDefaults(
        wxT("workunit tab area background"), (const char**)workunit_tab_area_background_image_xpm
    );
    m_WorkunitAreaBackgroundImage.SetDefaults(
        wxT("workunit area background"), (const char**)workunit_area_background_image_xpm
    );
    m_WorkunitAnimationBackgroundImage.SetDefaults(
        wxT("workunit animation background"), (const char**)workunit_animation_background_image_xpm
    );
    m_WorkunitAnimationImage.SetDefaults(
        wxT("workunit animation"), (const char**)workunit_animation_image_xpm
    );
    m_WorkunitGaugeBackgroundImage.SetDefaults(
        wxT("gauge background"), (const char**)workunit_gauge_background_image_xpm
    );
    m_WorkunitGaugeProgressIndicatorImage.SetDefaults(
        wxT("gauge progress indicator"), (const char**)workunit_gauge_progress_indicator_image_xpm
    );
    m_ProjectAreaBackgroundImage.SetDefaults(
        wxT("project area background"), (const char**)project_area_background_image_xpm
    );
    m_ProjectImage.SetDefaults(
        wxT("project"), (const char**)project_image_xpm
    );
    m_AttachProjectButton.SetDefaults(
        wxT("attach project"), (const char**)attach_project_button_xpm, (const char**)attach_project_clicked_button_xpm
    );
    m_HelpButton.SetDefaults(
        wxT("help"), (const char**)help_button_xpm, (const char**)help_clicked_button_xpm
    );
    m_RightArrowButton.SetDefaults(
        wxT("right arrow"), (const char**)right_arrow_button_xpm, (const char**)right_arrow_clicked_button_xpm
    );
    m_LeftArrowButton.SetDefaults(
        wxT("left arrow"), (const char**)left_arrow_button_xpm, (const char**)left_arrow_clicked_button_xpm
    );
    m_SaveButton.SetDefaults(
        wxT("save"), (const char**)save_button_xpm, (const char**)save_clicked_button_xpm
    );
    m_SynchronizeButton.SetDefaults(
        wxT("synchronize"), (const char**)synchronize_button_xpm, (const char**)synchronize_clicked_button_xpm
    );
    m_CancelButton.SetDefaults(
        wxT("cancel"), (const char**)cancel_button_xpm, (const char**)cancel_clicked_button_xpm
    );
    m_CloseButton.SetDefaults(
        wxT("close"), (const char**)close_button_xpm, (const char**)close_clicked_button_xpm
    );
    m_CopyAllButton.SetDefaults(
        wxT("copy all"), (const char**)copy_all_button_xpm, (const char**)copy_all_clicked_button_xpm
    );
    m_CopyButton.SetDefaults(
        wxT("copy"), (const char**)copy_button_xpm, (const char**)copy_clicked_button_xpm
    );
    m_MessagesLink.SetDefaults(
        wxT("messages link"), (const char**)messages_link_image_xpm
    );
    m_MessagesAlertLink.SetDefaults(
        wxT("messages alert link"), (const char**)messages_alert_link_image_xpm
    );
    m_SuspendLink.SetDefaults(
        wxT("suspend link"), (const char**)suspend_link_image_xpm
    );
    m_ResumeLink.SetDefaults(
        wxT("resume link"), (const char**)resume_link_image_xpm
    );
    m_PreferencesLink.SetDefaults(
        wxT("preferences link"), (const char**)preferences_link_image_xpm
    );
    m_AdvancedLink.SetDefaults(
        wxT("advanced link"), (const char**)advanced_link_image_xpm
    );
    m_DialogBackgroundImage.SetDefaults(
        wxT("dialog background"), (const char**)dialog_background_image_xpm
    );
    return true;
}


IMPLEMENT_DYNAMIC_CLASS(CSkinAdvanced, CSkinItem)


CSkinAdvanced::CSkinAdvanced() {
    Clear();
}


CSkinAdvanced::~CSkinAdvanced() {
    Clear();
}


void CSkinAdvanced::Clear() {
    m_bIsBranded = false;
    m_strApplicationName = wxEmptyString;
    m_strApplicationShortName = wxEmptyString;
    m_iconApplicationIcon.Clear();
    m_iconApplicationIcon32.Clear();
    m_iconApplicationDisconnectedIcon.Clear();
    m_iconApplicationSnoozeIcon.Clear();
    m_bitmapApplicationLogo = wxNullBitmap;
    m_strOrganizationName = wxEmptyString;
    m_strOrganizationWebsite = wxEmptyString;
    m_strOrganizationHelpUrl = wxEmptyString;
    m_bDefaultTabSpecified = false;
    m_iDefaultTab = 0;
    m_strExitMessage = wxEmptyString;
}


int CSkinAdvanced::Parse(MIOFILE& in) {
    char buf[256];
    std::string strBuffer;

    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</advanced>")) break;
        else if (parse_bool(buf, "is_branded", m_bIsBranded)) continue;
        else if (parse_str(buf, "<application_name>", strBuffer)) {
            m_strApplicationName = wxString(strBuffer.c_str(), wxConvUTF8);
            continue;
        } else if (parse_str(buf, "<application_short_name>", strBuffer)) {
            m_strApplicationShortName = wxString(strBuffer.c_str(), wxConvUTF8);
            continue;
        } else if (match_tag(buf, "<application_icon>")) {
            m_iconApplicationIcon.Parse(in);
            continue;
        } else if (match_tag(buf, "<application_icon32>")) {
            m_iconApplicationIcon32.Parse(in);
            continue;
        } else if (match_tag(buf, "<application_disconnected_icon>")) {
            m_iconApplicationDisconnectedIcon.Parse(in);
            continue;
        } else if (match_tag(buf, "<application_snooze_icon>")) {
            m_iconApplicationSnoozeIcon.Parse(in);
            continue;
        } else if (parse_str(buf, "<application_logo>", strBuffer)) {
            if(strBuffer.length()) {
                wxString str = wxString(
                    wxGetApp().GetSkinManager()->ConstructSkinPath() +
                    wxString(strBuffer.c_str(), wxConvUTF8)
                );
                m_bitmapApplicationLogo = wxBitmap(wxImage(str.c_str(), wxBITMAP_TYPE_ANY));
            }
            continue;
        } else if (parse_str(buf, "<organization_name>", strBuffer)) {
            m_strOrganizationName = wxString(strBuffer.c_str(), wxConvUTF8);
            continue;
        } else if (parse_str(buf, "<organization_website>", strBuffer)) {
            m_strOrganizationWebsite = wxString(strBuffer.c_str(), wxConvUTF8);
            continue;
        } else if (parse_str(buf, "<organization_help_url>", strBuffer)) {
            m_strOrganizationHelpUrl = wxString(strBuffer.c_str(), wxConvUTF8);
            continue;
        } else if (parse_int(buf, "<open_tab>", m_iDefaultTab)) {
            m_bDefaultTabSpecified = true;
            continue;
        } else if (parse_str(buf, "<exit_message>", strBuffer)) {
            m_strExitMessage = wxString(strBuffer.c_str(), wxConvUTF8);
            continue;
        }
    }

    InitializeDelayedValidation();

    return 0;
}


wxString CSkinAdvanced::GetApplicationName() {
    wxString strApplicationName = m_strApplicationName;
#ifdef BOINC_PRERELEASE
        strApplicationName += wxT(" (Pre-release)");
#endif
    return strApplicationName;
}


wxString CSkinAdvanced::GetApplicationShortName() {
    return m_strApplicationShortName;
}


wxIcon* CSkinAdvanced::GetApplicationIcon() {
    return m_iconApplicationIcon.GetIcon();
}

wxIcon* CSkinAdvanced::GetApplicationIcon32() {
    return m_iconApplicationIcon32.GetIcon();
}

wxIcon* CSkinAdvanced::GetApplicationDisconnectedIcon() { 
    return m_iconApplicationDisconnectedIcon.GetIcon();
}


wxIcon* CSkinAdvanced::GetApplicationSnoozeIcon() {
    return m_iconApplicationSnoozeIcon.GetIcon();
}


wxBitmap* CSkinAdvanced::GetApplicationLogo() {
    return &m_bitmapApplicationLogo;
}


wxString CSkinAdvanced::GetOrganizationName() { 
    return m_strOrganizationName;
}


wxString CSkinAdvanced::GetOrganizationWebsite() {
    return m_strOrganizationWebsite;
}


wxString CSkinAdvanced::GetOrganizationHelpUrl() {
    return m_strOrganizationHelpUrl;
}


int CSkinAdvanced::GetDefaultTab() { 
    return m_iDefaultTab;
}


wxString CSkinAdvanced::GetExitMessage() { 
    return m_strExitMessage;
}


bool CSkinAdvanced::IsBranded() { 
    return m_bIsBranded;
}


bool CSkinAdvanced::IsDefaultTabSpecified() { 
    return m_bDefaultTabSpecified;
}


bool CSkinAdvanced::InitializeDelayedValidation() {
    if (m_strApplicationName.IsEmpty()) {
        if (show_error_msgs) {
            fprintf(stderr, "Skin Manager: Application name was not defined. Using default.\n");
        }
        m_strApplicationName = wxT("BOINC Manager");
        wxASSERT(!m_strApplicationName.IsEmpty());
    }
    if (m_strApplicationShortName.IsEmpty()) {
        if (show_error_msgs) {
            fprintf(stderr, "Skin Manager: Application short name was not defined. Using default.\n");
        }
        m_strApplicationShortName = wxT("BOINC");
        wxASSERT(!m_strApplicationShortName.IsEmpty());
    }
    m_iconApplicationIcon.SetDefaults(wxT("application"), (const char**)boinc_xpm);
    m_iconApplicationIcon32.SetDefaults(wxT("application"), (const char**)boinc32_xpm);
    m_iconApplicationDisconnectedIcon.SetDefaults(wxT("application disconnected"), (const char**)boincdisconnect_xpm);
    m_iconApplicationSnoozeIcon.SetDefaults(wxT("application snooze"), (const char**)boincsnooze_xpm);
    if (!m_bitmapApplicationLogo.Ok()) {
        if (show_error_msgs) {
            fprintf(stderr, "Skin Manager: Failed to load application logo. Using default.\n");
        }
        m_bitmapApplicationLogo = wxBitmap((const char**)boinc_logo_xpm);
        wxASSERT(m_bitmapApplicationLogo.Ok());
    }
    if (m_strOrganizationName.IsEmpty()) {
        if (show_error_msgs) {
            fprintf(stderr, "Skin Manager: Organization name was not defined. Using default.\n");
        }
        m_strOrganizationName = wxT("Space Sciences Laboratory, U.C. Berkeley");
        wxASSERT(!m_strOrganizationName.IsEmpty());
    }
    if (m_strOrganizationWebsite.IsEmpty()) {
        if (show_error_msgs) {
            fprintf(stderr, "Skin Manager: Organization web site was not defined. Using default.\n");
        }
        m_strOrganizationWebsite = wxT("http://boinc.berkeley.edu");
        wxASSERT(!m_strOrganizationWebsite.IsEmpty());
    }
    if (m_strOrganizationHelpUrl.IsEmpty()) {
        if (show_error_msgs) {
            fprintf(stderr, "Skin Manager: Organization help url was not defined. Using default.\n");
        }
        m_strOrganizationHelpUrl = wxT("http://boinc.berkeley.edu/manager_links.php");
        wxASSERT(!m_strOrganizationHelpUrl.IsEmpty());
    }
    if (!m_bDefaultTabSpecified) {
        if (show_error_msgs) {
            fprintf(stderr, "Skin Manager: Default tab was not defined. Using default.\n");
        }
        m_bDefaultTabSpecified = true;
        m_iDefaultTab = 0;
    }
    if (m_strExitMessage.IsEmpty()) {
        if (show_error_msgs) {
            fprintf(stderr, "Skin Manager: Exit message was not defined. Using default.\n");
        }
        m_strExitMessage = wxEmptyString;
    }
    return true;
}


IMPLEMENT_DYNAMIC_CLASS(CSkinWizardATP, CSkinItem)


CSkinWizardATP::CSkinWizardATP() {
    Clear();
}


CSkinWizardATP::~CSkinWizardATP() {
    Clear();
}


void CSkinWizardATP::Clear() {
    m_bitmapWizardBitmap = wxNullBitmap;
    m_strTitle = wxEmptyString;
}


int CSkinWizardATP::Parse(MIOFILE& in) {
    char buf[256];
    std::string strBuffer;

    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</attach_to_project>")) break;
        else if (parse_str(buf, "<title>", strBuffer)) {
            m_strTitle = wxString(strBuffer.c_str(), wxConvUTF8);
            continue;
        } else if (parse_str(buf, "<logo>", strBuffer)) {
            if (strBuffer.length()) {
                wxString str = wxString(
                    wxGetApp().GetSkinManager()->ConstructSkinPath() +
                    wxString(strBuffer.c_str(), wxConvUTF8)
                );
                m_bitmapWizardBitmap = wxBitmap(wxImage(str.c_str(), wxBITMAP_TYPE_ANY));
            }
            continue;
        }
    }

    InitializeDelayedValidation();

    return 0;
}


bool CSkinWizardATP::InitializeDelayedValidation() {
    if (!m_bitmapWizardBitmap.Ok()) {
        if (show_error_msgs) {
            fprintf(stderr, "Skin Manager: Failed to load attach to project wizard bitmap logo. Using default.\n");
        }
        m_bitmapWizardBitmap = wxBitmap((const char**)wizard_bitmap_xpm);
        wxASSERT(m_bitmapWizardBitmap.Ok());
    }
    if (m_strTitle.IsEmpty()) {
        if (show_error_msgs) {
            fprintf(stderr, "Skin Manager: Attach to project wizard title was not defined. Using default.\n");
        }
        m_strTitle = wxT("BOINC Manager");
        wxASSERT(!m_strTitle.IsEmpty());
    }
    return true;
}


IMPLEMENT_DYNAMIC_CLASS(CSkinWizardATAM, CSkinItem)


CSkinWizardATAM::CSkinWizardATAM() {
    Clear();
}


CSkinWizardATAM::~CSkinWizardATAM() {
    Clear();
}


void CSkinWizardATAM::Clear() {
    m_bitmapWizardBitmap = wxNullBitmap;
    m_strTitle = wxEmptyString;
    m_strAccountInfoMessage = wxEmptyString;
}


int CSkinWizardATAM::Parse(MIOFILE& in) {
    char buf[256];
    std::string strBuffer;

    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</attach_to_account_manager>")) break;
        else if (parse_str(buf, "<title>", strBuffer)) {
            m_strTitle = wxString(strBuffer.c_str(), wxConvUTF8);
            continue;
        } else if (parse_str(buf, "<logo>", strBuffer)) {
            if(strBuffer.length()) {
                wxString str = wxString(
                    wxGetApp().GetSkinManager()->ConstructSkinPath() +
                    wxString(strBuffer.c_str(), wxConvUTF8)
                );
                m_bitmapWizardBitmap = wxBitmap(wxImage(str.c_str(), wxBITMAP_TYPE_ANY));
            }
            continue;
        } else if (parse_str(buf, "<account_info_message>", strBuffer)) {
            m_strAccountInfoMessage = wxString(strBuffer.c_str(), wxConvUTF8);
            continue;
        }
    }

    InitializeDelayedValidation();

    return 0;
}


bool CSkinWizardATAM::InitializeDelayedValidation() {
    if (!m_bitmapWizardBitmap.Ok()) {
        if (show_error_msgs) {
            fprintf(stderr, "Skin Manager: Failed to load attach to project wizard bitmap logo. Using default.\n");
        }
        m_bitmapWizardBitmap = wxBitmap((const char**)wizard_bitmap_xpm);
        wxASSERT(m_bitmapWizardBitmap.Ok());
    }
    if (m_strTitle.IsEmpty()) {
        if (show_error_msgs) {
            fprintf(stderr, "Skin Manager: Attach to project wizard title was not defined. Using default.\n");
        }
        m_strTitle = wxT("BOINC Manager");
        wxASSERT(!m_strTitle.IsEmpty());
    }
    return true;
}


IMPLEMENT_DYNAMIC_CLASS(CSkinWizards, CSkinItem)


CSkinWizards::CSkinWizards() {
    Clear();
}


CSkinWizards::~CSkinWizards() {
    Clear();
}


void CSkinWizards::Clear() {
    m_AttachToProjectWizard.Clear();
    m_AttachToAccountManagerWizard.Clear();
}


int CSkinWizards::Parse(MIOFILE& in) {
    char buf[256];

    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</wizards>")) break;
        else if (match_tag(buf, "<attach_to_project>")) {
            m_AttachToProjectWizard.Parse(in);
            continue;
        } else if (match_tag(buf, "<attach_to_account_manager>")) {
            m_AttachToAccountManagerWizard.Parse(in);
            continue;
        }
    }

    InitializeDelayedValidation();

    return 0;
}


bool CSkinWizards::InitializeDelayedValidation() {
    return m_AttachToProjectWizard.InitializeDelayedValidation() && 
           m_AttachToAccountManagerWizard.InitializeDelayedValidation();
}


IMPLEMENT_DYNAMIC_CLASS(CSkinManager, CSkinItem)


CSkinManager::CSkinManager() {}


CSkinManager::CSkinManager(bool debugSkins) {
    show_error_msgs = debugSkins;
    Clear();
}


CSkinManager::~CSkinManager() {
    Clear();
}

bool CSkinManager::ReloadSkin(wxLocale* pLocale, wxString strSkin) {
    int      retval;
    FILE*    p;
    MIOFILE  mf;
    wxString strDesiredLocale = pLocale->GetCanonicalName();

    // This fixes a (rare) crash bug
    if (strSkin.IsEmpty()) {
        strSkin = GetDefaultSkinName();
    }
    
    // Check to make sure that we are not trying to load the skin we
    //   are already using.
    if (m_strSelectedSkin == strSkin) return true;

    // Clear out all the old stuff 
    Clear();

    // Set the default skin back to Default
    m_strSelectedSkin = strSkin;

    // Check to see if the skin we want to change to is the default skin
    if (GetDefaultSkinName() == m_strSelectedSkin) {

        // Disable the error messages since the default images are
        //   going to be used.
        show_error_msgs = false;

        // Validate settings
        InitializeDelayedValidation();

        // Tell whichever UI elements that are loaded to reload the
        //   skinable resources they use.
        wxGetApp().FireReloadSkin();

        return true;
    }

    // First we try the users canonical locale resources.
    //  i.e. en_US
    retval = ERR_XML_PARSE;
    p = fopen((const char*)ConstructSkinFileName().mb_str(wxConvUTF8), "r");
    if (p) {
        mf.init_file(p);
        retval = Parse(mf, strDesiredLocale);
        fclose(p);
    }

    // If we failed the first lookup try the root language of the desired
    //   locale.
    //  i.e. en
    if (ERR_XML_PARSE == retval) {
        p = fopen((const char*)ConstructSkinFileName().mb_str(wxConvUTF8), "r");
        if (p) {
            mf.init_file(p);
            retval = Parse(mf, strDesiredLocale.Left(2));
            fclose(p);
        }
    }

    // If we failed the second lookup try english
    //  i.e. en
    if (ERR_XML_PARSE == retval) {
        p = fopen((const char*)ConstructSkinFileName().mb_str(wxConvUTF8), "r");
        if (p) {
            mf.init_file(p);
            retval = Parse(mf, wxT("en"));
            fclose(p);
        }
    }

    if (retval) {
        fprintf(stderr, "Skin Manager: Failed to load skin '%s'.\n", (char *)ConstructSkinFileName().c_str());
    }

    InitializeDelayedValidation();

    // Tell whichever UI elements that are loaded to reload the
    //   skinable resources they use.
    wxGetApp().FireReloadSkin();

    return true;
}

wxArrayString& CSkinManager::GetCurrentSkins() {
    unsigned int i;
    wxString     strSkinLocation = wxString(GetSkinsLocation() + wxFileName::GetPathSeparator());
    wxString     strSkinFileName = wxString(wxFileName::GetPathSeparator() + GetSkinFileName());

    // Initialize array
    m_astrSkins.Clear();

    // Go get all the valid skin directories.
    wxDir::GetAllFiles(strSkinLocation, &m_astrSkins, wxString(wxT("*") + GetSkinFileName()));

    // Trim out the path information for all the entries
    for (i = 0; i < m_astrSkins.GetCount(); i++) {
        m_astrSkins[i] = 
            m_astrSkins[i].Remove(0, strSkinLocation.Length());
        m_astrSkins[i] = 
            m_astrSkins[i].Remove(m_astrSkins[i].Find(strSkinFileName.c_str()), strSkinFileName.Length());

        // Special case: 'Default' to mean the embedded default skin.
        //   remove any duplicate entries
        if (GetDefaultSkinName() == m_astrSkins[i]) {
            m_astrSkins.RemoveAt(i);
        }
    }

    // Insert the 'Default' entry into the skins list.
    m_astrSkins.Insert(GetDefaultSkinName(), 0);

    // return the current list of skins
    return m_astrSkins;
}


wxString CSkinManager::GetDefaultSkinName() {
    return wxString(wxT("Default"));
}


wxString CSkinManager::ConstructSkinFileName() {
    return wxString(
        GetSkinsLocation() + 
        wxString(wxFileName::GetPathSeparator()) +
        m_strSelectedSkin + 
        wxString(wxFileName::GetPathSeparator()) +
        GetSkinFileName()
    );
}


wxString CSkinManager::ConstructSkinPath() {
    return wxString(
        GetSkinsLocation() + 
        wxString(wxFileName::GetPathSeparator()) +
        m_strSelectedSkin + 
        wxString(wxFileName::GetPathSeparator())
    );
}


wxString CSkinManager::GetSkinFileName() {
    // Construct path to skins directory
    return wxString(wxT("skin.xml"));
}


wxString CSkinManager::GetSkinsLocation() {
    // Construct path to skins directory
    wxString strSkinLocation = wxEmptyString;

#ifdef __WXMSW__
    strSkinLocation  = wxGetApp().GetRootDirectory();
    strSkinLocation += wxFileName::GetPathSeparator();
    strSkinLocation += wxT("skins");
#else
    strSkinLocation = wxString(wxGetCwd() + wxString(wxFileName::GetPathSeparator()) + wxT("skins"));
#endif

    return strSkinLocation;
}


void CSkinManager::Clear() {
    m_SimpleSkin.Clear();
    m_AdvancedSkin.Clear();
    m_WizardsSkin.Clear();

    m_astrSkins.Clear();
    m_strSelectedSkin.Clear();
}


int CSkinManager::Parse(MIOFILE& in, wxString strDesiredLocale) {
    char     buf[256];
    wxString strLocaleStartTag;
    wxString strLocaleEndTag;
    bool     bLocaleFound = false;

    // Construct the start and end tags for the locale we want.
    strLocaleStartTag.Printf(wxT("<%s>"), strDesiredLocale.c_str());
    strLocaleEndTag.Printf(wxT("</%s>"), strDesiredLocale.c_str());

    // Look for the begining of the desired locale.
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, (const char*)strLocaleStartTag.mb_str(wxConvUTF8))) {
            bLocaleFound = true;
            break;
        }
    }

    if (!bLocaleFound) return ERR_XML_PARSE;

    while (in.fgets(buf, 256)) {
        if (match_tag(buf, (const char*)strLocaleStartTag.mb_str(wxConvUTF8))) break;
        else if (match_tag(buf, "<simple>")) {
            m_SimpleSkin.Parse(in);
            continue;
        } else if (match_tag(buf, "<advanced>")) {
            m_AdvancedSkin.Parse(in);
            continue;
        } else if (match_tag(buf, "<wizards>")) {
            m_WizardsSkin.Parse(in);
            continue;
        }
    }

    InitializeDelayedValidation();

    return 0;
}


bool CSkinManager::InitializeDelayedValidation() {
    return m_SimpleSkin.InitializeDelayedValidation() && 
           m_AdvancedSkin.InitializeDelayedValidation() && 
           m_WizardsSkin.InitializeDelayedValidation();
}


