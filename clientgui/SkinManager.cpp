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
#include "res/skins/default/graphic/right_arrow_button.xpm"
#include "res/skins/default/graphic/right_arrow_clicked_button.xpm"
#include "res/skins/default/graphic/left_arrow_button.xpm"
#include "res/skins/default/graphic/left_arrow_clicked_button.xpm"
#include "res/skins/default/graphic/save_button.xpm"
#include "res/skins/default/graphic/save_clicked_button.xpm"
#include "res/skins/default/graphic/cancel_button.xpm"
#include "res/skins/default/graphic/cancel_clicked_button.xpm"
#include "res/skins/default/graphic/change_button.xpm"
#include "res/skins/default/graphic/change_clicked_button.xpm"
#include "res/skins/default/graphic/close_button.xpm"
#include "res/skins/default/graphic/close_clicked_button.xpm"
#include "res/skins/default/graphic/clear_button.xpm"
#include "res/skins/default/graphic/clear_clicked_button.xpm"
#include "res/skins/default/graphic/messages_link_image.xpm"
#include "res/skins/default/graphic/messages_alert_link_image.xpm"
#include "res/skins/default/graphic/suspend_link_image.xpm"
#include "res/skins/default/graphic/resume_link_image.xpm"
#include "res/skins/default/graphic/preferences_link_image.xpm"
#include "res/skins/default/graphic/advanced_link_image.xpm"
#include "res/skins/default/graphic/preferences_dialog_background_image.xpm"
#include "res/skins/default/graphic/messages_dialog_background_image.xpm"
#ifdef __APPLE__
#include "res/boinc_mac.xpm"
#else
#include "res/boinc.xpm"
#endif
#include "res/boincdisconnect.xpm"
#include "res/boincsnooze.xpm"
#include "res/boinc_logo.xpm"
#include "res/wizard_bitmap.xpm"
////@end XPM images


IMPLEMENT_DYNAMIC_CLASS(CSkinItem, wxObject)


CSkinItem::CSkinItem() {
}


CSkinItem::~CSkinItem() {
}


wxColour CSkinItem::ParseColor(wxString strColor) {
    int red, green, blue;
    wxStringTokenizer tkz(strColor, wxT(":"), wxTOKEN_RET_EMPTY);
    wxString(tkz.GetNextToken().mb_str()).ToLong((long*)&red);
	wxString(tkz.GetNextToken().mb_str()).ToLong((long*)&green);
	wxString(tkz.GetNextToken().mb_str()).ToLong((long*)&blue);
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
    m_bmpBitmap = wxNullBitmap;
    m_colBackgroundColor = wxNullColour;
    m_colTransparencyMask = wxNullColour;
}


bool CSkinImage::Ok() {
    return m_bmpBitmap.Ok();
}


int CSkinImage::Parse(MIOFILE& in) {
    char buf[256];
    std::string strBuffer;

    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</image>")) break;
        else if (parse_str(buf, "<imagesrc>", strBuffer)) {
            if (strBuffer.length()) {
                wxString str = wxString(
                    wxGetApp().GetSkinManager()->ConstructSkinPath() +
                    wxString(strBuffer.c_str(), wxConvUTF8)
                );
                m_bmpBitmap = wxBitmap(wxImage(str.c_str(), wxBITMAP_TYPE_ANY));
            }
            continue;
        } else if (parse_str(buf, "<background_color>", strBuffer)) {
            m_colBackgroundColor = ParseColor(wxString(strBuffer.c_str(), wxConvUTF8));
            continue;
        } else if (parse_str(buf, "<transparency_mask>", strBuffer)) {
            m_colTransparencyMask = ParseColor(wxString(strBuffer.c_str(), wxConvUTF8));
            continue;
        }
    }

    if (Ok()) {
        return 0;
    }

    return ERR_XML_PARSE;
}


bool CSkinImage::SetDefaults(const char** default_image) {
    m_bmpBitmap = wxBitmap(default_image);
    return true;
}


bool CSkinImage::SetDefaults(const char** default_image, wxString strBackgroundColor, wxString strTransparenyMask) {
    m_bmpBitmap = wxBitmap(default_image);
    m_colBackgroundColor = ParseColor(strBackgroundColor);
    m_colTransparencyMask = ParseColor(strTransparenyMask);
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
    m_bmpBitmap = wxNullBitmap;
    m_bmpBitmapClicked = wxNullBitmap;
}


bool CSkinSimpleButton::Ok() {
    return m_bmpBitmap.Ok() && m_bmpBitmapClicked.Ok();
}


int CSkinSimpleButton::Parse(MIOFILE& in) {
    char buf[256];
    std::string strBuffer;

    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</button>")) break;
        else if (parse_str(buf, "<imagesrc>", strBuffer)) {
            if (strBuffer.length()) {
                wxString str = wxString(
                    wxGetApp().GetSkinManager()->ConstructSkinPath() +
                    wxString(strBuffer.c_str(), wxConvUTF8)
                );
                m_bmpBitmap = wxBitmap(wxImage(str.c_str(), wxBITMAP_TYPE_ANY));
            }
            continue;
        }
        else if (parse_str(buf, "<imagesrc_clicked>", strBuffer)) {
            if (strBuffer.length()) {
                wxString str = wxString(
                    wxGetApp().GetSkinManager()->ConstructSkinPath() +
                    wxString(strBuffer.c_str(), wxConvUTF8)
                );
                m_bmpBitmapClicked = wxBitmap(wxImage(str.c_str(), wxBITMAP_TYPE_ANY));
            }
            continue;
        }
    }

    if (Ok()) {
        return 0;
    }

    return ERR_XML_PARSE;
}


bool CSkinSimpleButton::SetDefaults(const char** default_image, const char** default_clicked_image) {
    m_bmpBitmap = wxBitmap(default_image);
    m_bmpBitmapClicked = wxBitmap(default_clicked_image);
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
    m_bmpBitmap = wxNullBitmap;
    m_colBorderColor = wxNullColour;
    m_colGradientFromColor = wxNullColour;
    m_colGradientToColor = wxNullColour;
}


bool CSkinSimpleTab::Ok() {
    return m_bmpBitmap.Ok() && m_colBorderColor.Ok() && m_colGradientFromColor.Ok() && m_colGradientToColor.Ok();
}


int CSkinSimpleTab::Parse(MIOFILE& in) {
    char buf[256];
    std::string strBuffer;

    while (in.fgets(buf, 256)) {
        if (match_tag(buf, "</tab>")) break;
        else if (parse_str(buf, "<imagesrc>", strBuffer)) {
            if (strBuffer.length()) {
                wxString str = wxString(
                    wxGetApp().GetSkinManager()->ConstructSkinPath() +
                    wxString(strBuffer.c_str(), wxConvUTF8)
                );
                m_bmpBitmap = wxBitmap(wxImage(str.c_str(), wxBITMAP_TYPE_ANY));
            }
            continue;
        }
        else if (parse_str(buf, "<border_color>", strBuffer)) {
            m_colBorderColor = ParseColor(wxString(strBuffer.c_str(), wxConvUTF8));
            continue;
        }
        else if (parse_str(buf, "<gradient_from_color>", strBuffer)) {
            m_colGradientFromColor = ParseColor(wxString(strBuffer.c_str(), wxConvUTF8));
            continue;
        }
        else if (parse_str(buf, "<gradient_to_color>", strBuffer)) {
            m_colGradientToColor = ParseColor(wxString(strBuffer.c_str(), wxConvUTF8));
            continue;
        }
    }

    if (Ok()) {
        return 0;
    }

    return ERR_XML_PARSE;
}


bool CSkinSimpleTab::SetDefaults(
    const char** default_image, wxString strBorderColor, wxString strGradientFromColor, wxString strGradientToColor
) {
    m_bmpBitmap = wxBitmap(default_image);
    m_colBorderColor = ParseColor(strBorderColor);
    m_colGradientFromColor = ParseColor(strGradientFromColor);
    m_colGradientToColor = ParseColor(strGradientToColor);
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
    m_CancelButton.Clear();
    m_ChangeButton.Clear();
    m_CloseButton.Clear();
    m_ClearButton.Clear();

    m_PreferencesDialogBackgroundImage.Clear();
    m_MessagesDialogBackgroundImage.Clear();

    m_MessagesLink.Clear();
    m_MessagesAlertLink.Clear();
    m_SuspendLink.Clear();
    m_ResumeLink.Clear();
    m_PreferencesLink.Clear();
    m_AdvancedLink.Clear();
}


bool CSkinSimple::Ok() {
    if (!m_BackgroundImage.Ok()) return false;
    if (!m_SpacerImage.Ok()) return false;
    if (!m_StaticLineColor.Ok()) return false;
    if (!m_StateIndicatorBackgroundImage.Ok()) return false;
    if (!m_ConnectingIndicatorImage.Ok()) return false;
    if (!m_ErrorIndicatorImage.Ok()) return false;
    if (!m_WorkunitActiveTab.Ok()) return false;
    if (!m_WorkunitSuspendedTab.Ok()) return false;
    if (!m_WorkunitTabAreaBackgroundImage.Ok()) return false;
    if (!m_WorkunitAreaBackgroundImage.Ok()) return false;
    if (!m_WorkunitAnimationBackgroundImage.Ok()) return false;
    if (!m_WorkunitAnimationImage.Ok()) return false;
    if (!m_WorkunitGaugeBackgroundImage.Ok()) return false;
    if (!m_WorkunitGaugeProgressIndicatorImage.Ok()) return false;
    if (!m_ProjectAreaBackgroundImage.Ok()) return false;
    if (!m_ProjectImage.Ok()) return false;
    if (!m_AttachProjectButton.Ok()) return false;
    if (!m_RightArrowButton.Ok()) return false;
    if (!m_LeftArrowButton.Ok()) return false;
    if (!m_SaveButton.Ok()) return false;
    if (!m_CancelButton.Ok()) return false;
    if (!m_CloseButton.Ok()) return false;
    if (!m_ChangeButton.Ok()) return false;
    if (!m_ClearButton.Ok()) return false;
    if (!m_MessagesLink.Ok()) return false;
    if (!m_MessagesAlertLink.Ok()) return false;
    if (!m_SuspendLink.Ok()) return false;
    if (!m_ResumeLink.Ok()) return false;
    if (!m_PreferencesLink.Ok()) return false;
    if (!m_AdvancedLink.Ok()) return false;
    if (!m_PreferencesDialogBackgroundImage.Ok()) return false;
    if (!m_MessagesDialogBackgroundImage.Ok()) return false;
    return true;
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
        } else if (match_tag(buf, "<right_arrow_button>")) {
            m_RightArrowButton.Parse(in);
            continue;
        } else if (match_tag(buf, "<left_arrow_button>")) {
            m_LeftArrowButton.Parse(in);
            continue;
        } else if (match_tag(buf, "<save_button>")) {
            m_SaveButton.Parse(in);
            continue;
        } else if (match_tag(buf, "<cancel_button>")) {
            m_CancelButton.Parse(in);
            continue;
        } else if (match_tag(buf, "<change_button>")) {
            m_ChangeButton.Parse(in);
            continue;
        } else if (match_tag(buf, "<close_button>")) {
            m_CloseButton.Parse(in);
            continue;
        } else if (match_tag(buf, "<clear_button>")) {
            m_ClearButton.Parse(in);
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
        } else if (match_tag(buf, "<preferences_dialog_background_image>")) {
            m_PreferencesDialogBackgroundImage.Parse(in);
            continue;
        } else if (match_tag(buf, "<messages_dialog_background_image>")) {
            m_MessagesDialogBackgroundImage.Parse(in);
            continue;
        }
    }

    // Make sure all of our parts and pieces are going to work if not replace them
    //   with the defaults.
    ValidateSkin();

    // Check one last time to make sure everything is good to go.
    //
    if (Ok()) {
        return 0;
    }

    return ERR_XML_PARSE;
}


bool CSkinSimple::ValidateSkin() {
    if (!m_BackgroundImage.Ok()) {
        if (!m_BackgroundImage.GetBitmap()->Ok()) {
            fprintf(stderr, wxT("Skin Manager: Failed to load background image. Using default.\n"));
        }
        if (!m_BackgroundImage.GetBackgroundColor()->Ok()) {
            fprintf(stderr, wxT("Skin Manager: Failed to parse background color. Using default.\n"));
        }
        m_BackgroundImage.SetDefaults((const char**)background_image_xpm, wxString(wxT("133:181:178")), wxEmptyString);
        wxASSERT(m_BackgroundImage.Ok());
    }
    if (!m_SpacerImage.Ok()) {
        fprintf(stderr, wxT("Skin Manager: Failed to load spacer image. Using default.\n"));
        m_SpacerImage.SetDefaults((const char**)spacer_image_xpm);
        wxASSERT(m_SpacerImage.Ok());
    }
    if (!m_StaticLineColor.Ok()) {
        fprintf(stderr, wxT("Skin Manager: Failed to parse static line color. Using default.\n"));
        m_StaticLineColor = ParseColor(wxString(wxT("204:102:51")));
        wxASSERT(m_StaticLineColor.Ok());
    }
    if (!m_StateIndicatorBackgroundImage.Ok()) {
        fprintf(stderr, wxT("Skin Manager: Failed to load state indicator background image. Using default.\n"));
        m_StateIndicatorBackgroundImage.SetDefaults((const char**)state_indicator_background_image_xpm);
        wxASSERT(m_StateIndicatorBackgroundImage.Ok());
    }
    if (!m_ConnectingIndicatorImage.Ok()) {
        fprintf(stderr, wxT("Skin Manager: Failed to load connecting indicator image. Using default.\n"));
        m_ConnectingIndicatorImage.SetDefaults((const char**)connecting_indicator_image_xpm);
        wxASSERT(m_ConnectingIndicatorImage.Ok());
    }
    if (!m_ErrorIndicatorImage.Ok()) {
        fprintf(stderr, wxT("Skin Manager: Failed to load error indicator image. Using default.\n"));
        m_ErrorIndicatorImage.SetDefaults((const char**)error_indicator_image_xpm);
        wxASSERT(m_ErrorIndicatorImage.Ok());
    }
    if (!m_WorkunitActiveTab.Ok()) {
        if (!m_WorkunitActiveTab.GetBitmap()->Ok()) {
            fprintf(stderr, wxT("Skin Manager: Failed to load active tab image. Using default.\n"));
        }
        if (!m_WorkunitActiveTab.GetBorderColor()->Ok()) {
            fprintf(stderr, wxT("Skin Manager: Failed to parse active tab border color. Using default.\n"));
        }
        if (!m_WorkunitActiveTab.GetGradientFromColor()->Ok()) {
            fprintf(stderr, wxT("Skin Manager: Failed to parse active tab gradient from color. Using default.\n"));
        }
        if (!m_WorkunitActiveTab.GetGradientToColor()->Ok()) {
            fprintf(stderr, wxT("Skin Manager: Failed to parse active tab gradient to color. Using default.\n"));
        }
        m_WorkunitActiveTab.SetDefaults(
            (const char**)workunit_active_image_xpm,
            wxString(wxT("20:82:82")),
            wxString(wxT("134:179:176")),
            wxString(wxT("51:102:102"))
        );
        wxASSERT(m_WorkunitActiveTab.Ok());
    }
    if (!m_WorkunitSuspendedTab.Ok()) {
        if (!m_WorkunitSuspendedTab.GetBitmap()->Ok()) {
            fprintf(stderr, wxT("Skin Manager: Failed to load suspended tab image. Using default.\n"));
        }
        if (!m_WorkunitSuspendedTab.GetBorderColor()->Ok()) {
            fprintf(stderr, wxT("Skin Manager: Failed to parse suspended tab border color. Using default.\n"));
        }
        if (!m_WorkunitSuspendedTab.GetGradientFromColor()->Ok()) {
            fprintf(stderr, wxT("Skin Manager: Failed to parse suspended tab gradient from color. Using default.\n"));
        }
        if (!m_WorkunitSuspendedTab.GetGradientToColor()->Ok()) {
            fprintf(stderr, wxT("Skin Manager: Failed to parse suspended tab gradient to color. Using default.\n"));
        }
        m_WorkunitSuspendedTab.SetDefaults(
            (const char**)workunit_suspended_image_xpm,
            wxString(wxT("102:153:153")),
            wxString(wxT("134:179:176")),
            wxString(wxT("84:175:175"))
        );
        wxASSERT(m_WorkunitSuspendedTab.Ok());
    }
    if (!m_WorkunitTabAreaBackgroundImage.Ok()) {
        fprintf(stderr, wxT("Skin Manager: Failed to load workunit tab area background image. Using default.\n"));
        m_WorkunitTabAreaBackgroundImage.SetDefaults((const char**)workunit_tab_area_background_image_xpm);
        wxASSERT(m_WorkunitTabAreaBackgroundImage.Ok());
    }
    if (!m_WorkunitAreaBackgroundImage.Ok()) {
        fprintf(stderr, wxT("Skin Manager: Failed to load workunit area background image. Using default.\n"));
        m_WorkunitAreaBackgroundImage.SetDefaults((const char**)workunit_area_background_image_xpm);
        wxASSERT(m_WorkunitAreaBackgroundImage.Ok());
    }
    if (!m_WorkunitAnimationBackgroundImage.Ok()) {
        fprintf(stderr, wxT("Skin Manager: Failed to load workunit animation background image. Using default.\n"));
        m_WorkunitAnimationBackgroundImage.SetDefaults((const char**)workunit_animation_background_image_xpm);
        wxASSERT(m_WorkunitAnimationBackgroundImage.Ok());
    }
    if (!m_WorkunitAnimationImage.Ok()) {
        fprintf(stderr, wxT("Skin Manager: Failed to load workunit animation image. Using default.\n"));
        m_WorkunitAnimationImage.SetDefaults((const char**)workunit_animation_image_xpm);
        wxASSERT(m_WorkunitAnimationImage.Ok());
    }
    if (!m_WorkunitGaugeBackgroundImage.Ok()) {
        fprintf(stderr, wxT("Skin Manager: Failed to load gauge background image. Using default.\n"));
        m_WorkunitGaugeBackgroundImage.SetDefaults((const char**)workunit_gauge_background_image_xpm);
        wxASSERT(m_WorkunitGaugeBackgroundImage.Ok());
    }
    if (!m_WorkunitGaugeProgressIndicatorImage.Ok()) {
        fprintf(stderr, wxT("Skin Manager: Failed to load gauge progress indicator image. Using default.\n"));
        m_WorkunitGaugeProgressIndicatorImage.SetDefaults((const char**)workunit_gauge_progress_indicator_image_xpm);
        wxASSERT(m_WorkunitGaugeProgressIndicatorImage.Ok());
    }
    if (!m_ProjectAreaBackgroundImage.Ok()) {
        fprintf(stderr, wxT("Skin Manager: Failed to load project area background image. Using default.\n"));
        m_ProjectAreaBackgroundImage.SetDefaults((const char**)project_area_background_image_xpm);
        wxASSERT(m_ProjectAreaBackgroundImage.Ok());
    }
    if (!m_ProjectImage.Ok()) {
        fprintf(stderr, wxT("Skin Manager: Failed to load project image. Using default.\n"));
        m_ProjectImage.SetDefaults((const char**)project_image_xpm);
        wxASSERT(m_ProjectImage.Ok());
    }
    if (!m_AttachProjectButton.Ok()) {
        if (!m_AttachProjectButton.GetBitmap()->Ok()) {
            fprintf(stderr, wxT("Skin Manager: Failed to load attach project button. Using default.\n"));
        }
        if (!m_AttachProjectButton.GetBitmapClicked()->Ok()) {
            fprintf(stderr, wxT("Skin Manager: Failed to load attach project clicked button. Using default.\n"));
        }
        m_AttachProjectButton.SetDefaults(
            (const char**)attach_project_button_xpm,
            (const char**)attach_project_clicked_button_xpm
        );
        wxASSERT(m_AttachProjectButton.Ok());
    }
    if (!m_RightArrowButton.Ok()) {
        if (!m_RightArrowButton.GetBitmap()->Ok()) {
            fprintf(stderr, wxT("Skin Manager: Failed to load right arrow button. Using default.\n"));
        }
        if (!m_RightArrowButton.GetBitmapClicked()->Ok()) {
            fprintf(stderr, wxT("Skin Manager: Failed to load right arrow clicked button. Using default.\n"));
        }
        m_RightArrowButton.SetDefaults(
            (const char**)right_arrow_button_xpm,
            (const char**)right_arrow_clicked_button_xpm
        );
        wxASSERT(m_RightArrowButton.Ok());
    }
    if (!m_LeftArrowButton.Ok()) {
        if (!m_LeftArrowButton.GetBitmap()->Ok()) {
            fprintf(stderr, wxT("Skin Manager: Failed to load left arrow button. Using default.\n"));
        }
        if (!m_LeftArrowButton.GetBitmapClicked()->Ok()) {
            fprintf(stderr, wxT("Skin Manager: Failed to load left arrow clicked button. Using default.\n"));
        }
        m_LeftArrowButton.SetDefaults(
            (const char**)left_arrow_button_xpm,
            (const char**)left_arrow_clicked_button_xpm
        );
        wxASSERT(m_LeftArrowButton.Ok());
    }
    if (!m_SaveButton.Ok()) {
        if (!m_SaveButton.GetBitmap()->Ok()) {
            fprintf(stderr, wxT("Skin Manager: Failed to load save button. Using default.\n"));
        }
        if (!m_SaveButton.GetBitmapClicked()->Ok()) {
            fprintf(stderr, wxT("Skin Manager: Failed to load save clicked button. Using default.\n"));
        }
        m_SaveButton.SetDefaults(
            (const char**)save_button_xpm,
            (const char**)save_clicked_button_xpm
        );
        wxASSERT(m_SaveButton.Ok());
    }
    if (!m_CancelButton.Ok()) {
        if (!m_CancelButton.GetBitmap()->Ok()) {
            fprintf(stderr, wxT("Skin Manager: Failed to load cancel button. Using default.\n"));
        }
        if (!m_CancelButton.GetBitmapClicked()->Ok()) {
            fprintf(stderr, wxT("Skin Manager: Failed to load cancel clicked button. Using default.\n"));
        }
        m_CancelButton.SetDefaults(
            (const char**)cancel_button_xpm,
            (const char**)cancel_clicked_button_xpm
        );
        wxASSERT(m_CancelButton.Ok());
    }
    if (!m_ChangeButton.Ok()) {
        if (!m_ChangeButton.GetBitmap()->Ok()) {
            fprintf(stderr, wxT("Skin Manager: Failed to load change button. Using default.\n"));
        }
        if (!m_ChangeButton.GetBitmapClicked()->Ok()) {
            fprintf(stderr, wxT("Skin Manager: Failed to load change clicked button. Using default.\n"));
        }
        m_ChangeButton.SetDefaults(
            (const char**)change_button_xpm,
            (const char**)change_clicked_button_xpm
        );
        wxASSERT(m_ChangeButton.Ok());
    }
    if (!m_CloseButton.Ok()) {
        if (!m_CloseButton.GetBitmap()->Ok()) {
            fprintf(stderr, wxT("Skin Manager: Failed to load close button. Using default.\n"));
        }
        if (!m_CloseButton.GetBitmapClicked()->Ok()) {
            fprintf(stderr, wxT("Skin Manager: Failed to load close clicked button. Using default.\n"));
        }
        m_CloseButton.SetDefaults(
            (const char**)close_button_xpm,
            (const char**)close_clicked_button_xpm
        );
        wxASSERT(m_CloseButton.Ok());
    }
    if (!m_ClearButton.Ok()) {
        if (!m_ClearButton.GetBitmap()->Ok()) {
            fprintf(stderr, wxT("Skin Manager: Failed to load clear button. Using default.\n"));
        }
        if (!m_ClearButton.GetBitmapClicked()->Ok()) {
            fprintf(stderr, wxT("Skin Manager: Failed to load clear clicked button. Using default.\n"));
        }
        m_ClearButton.SetDefaults(
            (const char**)clear_button_xpm,
            (const char**)clear_clicked_button_xpm
        );
        wxASSERT(m_ClearButton.Ok());
    }
    if (!m_MessagesLink.Ok()) {
        fprintf(stderr, wxT("Skin Manager: Failed to load messages link image. Using default.\n"));
        m_MessagesLink.SetDefaults((const char**)messages_link_image_xpm);
        wxASSERT(m_MessagesLink.Ok());
    }
    if (!m_MessagesAlertLink.Ok()) {
        fprintf(stderr, wxT("Skin Manager: Failed to load messages alert link image. Using default.\n"));
        m_MessagesAlertLink.SetDefaults((const char**)messages_alert_link_image_xpm);
        wxASSERT(m_MessagesAlertLink.Ok());
    }
    if (!m_SuspendLink.Ok()) {
        fprintf(stderr, wxT("Skin Manager: Failed to load suspend link image. Using default.\n"));
        m_SuspendLink.SetDefaults((const char**)suspend_link_image_xpm);
        wxASSERT(m_SuspendLink.Ok());
    }
    if (!m_ResumeLink.Ok()) {
        fprintf(stderr, wxT("Skin Manager: Failed to load resume link image. Using default.\n"));
        m_ResumeLink.SetDefaults((const char**)resume_link_image_xpm);
        wxASSERT(m_ResumeLink.Ok());
    }
    if (!m_PreferencesLink.Ok()) {
        fprintf(stderr, wxT("Skin Manager: Failed to load preferences link image. Using default.\n"));
        m_PreferencesLink.SetDefaults((const char**)preferences_link_image_xpm);
        wxASSERT(m_PreferencesLink.Ok());
    }
    if (!m_AdvancedLink.Ok()) {
        fprintf(stderr, wxT("Skin Manager: Failed to load advanced link image. Using default.\n"));
        m_AdvancedLink.SetDefaults((const char**)advanced_link_image_xpm);
        wxASSERT(m_AdvancedLink.Ok());
    }
    if (!m_PreferencesDialogBackgroundImage.Ok()) {
        fprintf(stderr, wxT("Skin Manager: Failed to load preferences dialog background image. Using default.\n"));
        m_PreferencesDialogBackgroundImage.SetDefaults((const char**)preferences_dialog_background_image_xpm);
        wxASSERT(m_PreferencesDialogBackgroundImage.Ok());
    }
    if (!m_MessagesDialogBackgroundImage.Ok()) {
        fprintf(stderr, wxT("Skin Manager: Failed to load messages dialog background image. Using default.\n"));
        m_MessagesDialogBackgroundImage.SetDefaults((const char**)messages_dialog_background_image_xpm);
        wxASSERT(m_MessagesDialogBackgroundImage.Ok());
    }
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
    m_iconApplicationIcon = wxNullIcon;
    m_iconApplicationDisconnectedIcon = wxNullIcon;
    m_iconApplicationSnoozeIcon = wxNullIcon;
    m_bitmapApplicationLogo = wxNullBitmap;
    m_strCompanyName = wxEmptyString;
    m_strCompanyWebsite = wxEmptyString;
    m_strProjectName = wxEmptyString;
    m_bDefaultTabSpecified = false;
    m_iDefaultTab = 0;
    m_strExitMessage = wxEmptyString;
}


bool CSkinAdvanced::Ok() {
    if (m_strApplicationName.IsEmpty()) return false;
    if (!m_iconApplicationIcon.Ok()) return false;
    if (!m_iconApplicationDisconnectedIcon.Ok()) return false;
    if (!m_iconApplicationSnoozeIcon.Ok()) return false;
    if (!m_bitmapApplicationLogo.Ok()) return false;
    if (m_strCompanyName.IsEmpty()) return false;
    if (m_strCompanyWebsite.IsEmpty()) return false;
    if (m_strProjectName.IsEmpty()) return false;
    if (!m_bDefaultTabSpecified) return false;
    if (m_strExitMessage.IsEmpty()) return false;
    return true;
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
        } else if (match_tag(buf, "<application_icon>")) {
            // Parse out the bitmap information and transparency mask
            CSkinImage img;
            img.Parse(in);

            if (img.Ok()) {
                // Configure bitmap object with transparency mask
                wxBitmap bmp = wxBitmap(*img.GetBitmap());
                bmp.SetMask(new wxMask(*img.GetBitmap(), *img.GetTransparencyMask()));

                // Not set the icon object using the newly created bitmap
                m_iconApplicationIcon.CopyFromBitmap(bmp);
            }
            continue;
        } else if (match_tag(buf, "<application_disconnected_icon>")) {
            // Parse out the bitmap information and transparency mask
            CSkinImage img;
            img.Parse(in);

            if (img.Ok()) {
                // Configure bitmap object with transparency mask
                wxBitmap bmp = wxBitmap(*img.GetBitmap());
                bmp.SetMask(new wxMask(*img.GetBitmap(), *img.GetTransparencyMask()));

                // Not set the icon object using the newly created bitmap
                m_iconApplicationDisconnectedIcon.CopyFromBitmap(bmp);
            }
            continue;
        } else if (match_tag(buf, "<application_snooze_icon>")) {
            // Parse out the bitmap information and transparency mask
            CSkinImage img;
            img.Parse(in);

            if (img.Ok()) {
                // Configure bitmap object with transparency mask
                wxBitmap bmp = wxBitmap(*img.GetBitmap());
                bmp.SetMask(new wxMask(*img.GetBitmap(), *img.GetTransparencyMask()));

                // Not set the icon object using the newly created bitmap
                m_iconApplicationSnoozeIcon.CopyFromBitmap(bmp);
            }
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
        } else if (parse_str(buf, "<company_name>", strBuffer)) {
            m_strCompanyName = wxString(strBuffer.c_str(), wxConvUTF8);
            continue;
        } else if (parse_str(buf, "<company_website>", strBuffer)) {
            m_strCompanyWebsite = wxString(strBuffer.c_str(), wxConvUTF8);
            continue;
        } else if (parse_str(buf, "<project_name>", strBuffer)) {
            m_strProjectName = wxString(strBuffer.c_str(), wxConvUTF8);
            continue;
        } else if (parse_int(buf, "<open_tab>", m_iDefaultTab)) {
            m_bDefaultTabSpecified = true;
            continue;
        } else if (parse_str(buf, "<exit_message>", strBuffer)) {
            m_strExitMessage = wxString(strBuffer.c_str(), wxConvUTF8);
            continue;
        }
    }

    // Make sure all of our parts and pieces are going to work if not replace them
    //   with the defaults.
    ValidateSkin();

    // Check one last time to make sure everything is good to go.
    //
    if (Ok()) {
        return 0;
    }

    return ERR_XML_PARSE;
}


bool CSkinAdvanced::ValidateSkin() {
    if (m_strApplicationName.IsEmpty()) {
        fprintf(stderr, wxT("Skin Manager: Application name was not defined. Using default.\n"));
        m_strApplicationName = wxT("BOINC Manager");
        wxASSERT(!m_strApplicationName.IsEmpty());
    }
    if (!m_iconApplicationIcon.Ok()) {
        fprintf(stderr, wxT("Skin Manager: Failed to load application icon. Using default.\n"));
        m_iconApplicationIcon = wxIcon((const char**)boinc_xpm);
        wxASSERT(m_iconApplicationIcon.Ok());
    }
    if (!m_iconApplicationDisconnectedIcon.Ok()) {
        fprintf(stderr, wxT("Skin Manager: Failed to load application disconnected icon. Using default.\n"));
        m_iconApplicationDisconnectedIcon = wxIcon((const char**)boincdisconnect_xpm);
        wxASSERT(m_iconApplicationDisconnectedIcon.Ok());
    }
    if (!m_iconApplicationSnoozeIcon.Ok()) {
        fprintf(stderr, wxT("Skin Manager: Failed to load application snooze icon. Using default.\n"));
        m_iconApplicationSnoozeIcon = wxIcon((const char**)boincsnooze_xpm);
        wxASSERT(m_iconApplicationSnoozeIcon.Ok());
    }
    if (!m_bitmapApplicationLogo.Ok()) {
        fprintf(stderr, wxT("Skin Manager: Failed to load application logo. Using default.\n"));
        m_bitmapApplicationLogo = wxBitmap((const char**)boinc_logo_xpm);
        wxASSERT(m_bitmapApplicationLogo.Ok());
    }
    if (m_strCompanyName.IsEmpty()) {
        fprintf(stderr, wxT("Skin Manager: Company name was not defined. Using default.\n"));
        m_strCompanyName = wxT("Space Sciences Laboratory, U.C. Berkeley");
        wxASSERT(!m_strCompanyName.IsEmpty());
    }
    if (m_strCompanyWebsite.IsEmpty()) {
        fprintf(stderr, wxT("Skin Manager: Company web site was not defined. Using default.\n"));
        m_strCompanyWebsite = wxT("http://boinc.berkeley.edu");
        wxASSERT(!m_strCompanyWebsite.IsEmpty());
    }
    if (m_strProjectName.IsEmpty()) {
        fprintf(stderr, wxT("Skin Manager: Project name was not defined. Using default.\n"));
        m_strProjectName = wxT("BOINC");
        wxASSERT(!m_strProjectName.IsEmpty());
    }
    if (!m_bDefaultTabSpecified) {
        fprintf(stderr, wxT("Skin Manager: Default tab was not defined. Using default.\n"));
        m_bDefaultTabSpecified = true;
        m_iDefaultTab = 0;
    }
    if (m_strExitMessage.IsEmpty()) {
        fprintf(stderr, wxT("Skin Manager: Exit message was not defined. Using default.\n"));
        m_strExitMessage = 
            _("This will shut down your tasks until it restarts automatically\n"
              "following your user preferences. Close window to close the manager\n"
              "without stopping the tasks.");
        wxASSERT(!m_strExitMessage.IsEmpty());
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


bool CSkinWizardATP::Ok() {
    if (!m_bitmapWizardBitmap.Ok()) return false;
    if (m_strTitle.IsEmpty()) return false;
    return true;
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

    // Make sure all of our parts and pieces are going to work if not replace them
    //   with the defaults.
    ValidateSkin();

    // Check one last time to make sure everything is good to go.
    //
    if (Ok()) {
        return 0;
    }

    return ERR_XML_PARSE;
}


bool CSkinWizardATP::ValidateSkin() {
    if (!m_bitmapWizardBitmap.Ok()) {
        fprintf(stderr, wxT("Skin Manager: Failed to load attach to project wizard bitmap logo. Using default.\n"));
        m_bitmapWizardBitmap = wxBitmap((const char**)wizard_bitmap_xpm);
        wxASSERT(m_bitmapWizardBitmap.Ok());
    }
    if (m_strTitle.IsEmpty()) {
        fprintf(stderr, wxT("Skin Manager: Attach to project wizard title was not defined. Using default.\n"));
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


bool CSkinWizardATAM::Ok() {
    if (!m_bitmapWizardBitmap.Ok()) return false;
    if (m_strTitle.IsEmpty()) return false;
    return true;
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

    // Make sure all of our parts and pieces are going to work if not replace them
    //   with the defaults.
    ValidateSkin();

    // Check one last time to make sure everything is good to go.
    //
    if (Ok()) {
        return 0;
    }

    return ERR_XML_PARSE;
}


bool CSkinWizardATAM::ValidateSkin() {
    if (!m_bitmapWizardBitmap.Ok()) {
        fprintf(stderr, wxT("Skin Manager: Failed to load attach to project wizard bitmap logo. Using default.\n"));
        m_bitmapWizardBitmap = wxBitmap((const char**)wizard_bitmap_xpm);
        wxASSERT(m_bitmapWizardBitmap.Ok());
    }
    if (m_strTitle.IsEmpty()) {
        fprintf(stderr, wxT("Skin Manager: Attach to project wizard title was not defined. Using default.\n"));
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


bool CSkinWizards::Ok() {
    if (!m_AttachToProjectWizard.Ok()) return false;
    if (!m_AttachToAccountManagerWizard.Ok()) return false;
    return true;
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

    // Make sure all of our parts and pieces are going to work if not replace them
    //   with the defaults.
    ValidateSkin();

    // Check one last time to make sure everything is good to go.
    //
    if (Ok()) {
        return 0;
    }

    return ERR_XML_PARSE;
}


bool CSkinWizards::ValidateSkin() {
    return m_AttachToProjectWizard.ValidateSkin() && m_AttachToAccountManagerWizard.ValidateSkin();
}


IMPLEMENT_DYNAMIC_CLASS(CSkinManager, CSkinItem)


CSkinManager::CSkinManager() {
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


    // Clear out all the old stuff 
    Clear();

    // Set the default skin back to Default
    m_strSelectedSkin = strSkin;

    // Check to see if the skin we want to change to is the default skin
    if (GetDefaultSkinName() == m_strSelectedSkin) {
        // Validate settings
        ValidateSkin();

        // Tell whichever frame is loaded to reload and skinable resources
        CBOINCBaseFrame* pFrame = wxGetApp().GetFrame();
        if (pFrame) {
	        pFrame->FireReloadSkin();
        }

        return true;
    }

    // First we try the users canonical locale resources.
    //  i.e. en_US
    retval = ERR_XML_PARSE;
    p = fopen(ConstructSkinFileName().c_str(), "r");
    if (p) {
        mf.init_file(p);
        retval = Parse(mf, strDesiredLocale);
        fclose(p);
    }

    // If we failed the first lookup try the root language of the desired
    //   locale.
    //  i.e. en
    if (ERR_XML_PARSE == retval) {
        p = fopen(ConstructSkinFileName().c_str(), "r");
        if (p) {
            mf.init_file(p);
            retval = Parse(mf, strDesiredLocale.Left(2));
            fclose(p);
        }
    }

    // If we failed the second lookup try english
    //  i.e. en
    if (ERR_XML_PARSE == retval) {
        p = fopen(ConstructSkinFileName().c_str(), "r");
        if (p) {
            mf.init_file(p);
            retval = Parse(mf, wxT("en"));
            fclose(p);
        }
    }

    if (retval) {
        fprintf(
            stderr,
            wxT("Skin Manager: Failed to load skin '%s'.\n"),
            ConstructSkinFileName().c_str()
        );
    }

    // Make sure all the resources are loaded and valid.
    ValidateSkin();

    // Tell whichever frame is loaded to reload and skinable resources
    CBOINCBaseFrame* pFrame = wxGetApp().GetFrame();
    if (pFrame) {
        pFrame->FireReloadSkin();
    }

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
    return wxString(wxGetCwd() + wxString(wxFileName::GetPathSeparator()) + wxT("skins"));
}


void CSkinManager::Clear() {
    m_SimpleSkin.Clear();
    m_AdvancedSkin.Clear();
    m_WizardsSkin.Clear();

    m_astrSkins.Clear();
    m_strSelectedSkin.Clear();
}


bool CSkinManager::Ok() {
    if (!m_SimpleSkin.Ok()) return false;
    if (!m_AdvancedSkin.Ok()) return false;
    if (!m_WizardsSkin.Ok()) return false;
    return true;
}


int CSkinManager::Parse(MIOFILE& in, wxString strDesiredLocale) {
    char     buf[256];
    wxString strLocaleStartTag;
    wxString strLocaleEndTag;
    bool     bLocaleFound = false;

    // Construct the start and end tags for the locale we want.
    strLocaleStartTag.Printf("<%s>", strDesiredLocale.c_str());
    strLocaleEndTag.Printf("</%s>", strDesiredLocale.c_str());

    // Look for the begining of the desired locale.
    while (in.fgets(buf, 256)) {
        if (match_tag(buf, strLocaleStartTag.c_str())) {
            bLocaleFound = true;
            break;
        }
    }

    if (!bLocaleFound) return ERR_XML_PARSE;

    while (in.fgets(buf, 256)) {
        if (match_tag(buf, strLocaleEndTag.c_str())) break;
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

    // Make sure all of our parts and pieces are going to work if not replace them
    //   with the defaults.
    ValidateSkin();

    // Check one last time to make sure everything is good to go.
    //
    if (Ok()) {
        return 0;
    }

    return ERR_XML_PARSE;
}


bool CSkinManager::ValidateSkin() {
    return m_SimpleSkin.ValidateSkin() && m_AdvancedSkin.ValidateSkin() && m_WizardsSkin.ValidateSkin();
}

