// This file is part of BOINC.
// https://boinc.berkeley.edu
// Copyright (C) 2026 University of California
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

#ifndef BOINC_SG_TASKPANEL_H
#define BOINC_SG_TASKPANEL_H

// Comment???
//
#define SELECTBYRESULTNAME 0

#include "sg_PanelBase.h"


typedef struct {
    RESULT * result;
    char result_name[256];
    char project_url[256];
    int dotColor;
    wxArrayString slideShowFileNames;
    int lastSlideShown;
    double project_files_downloaded_time;
} TaskSelectionData;



///////////////////////////////////////////////////////////////////////////
/// Class CScrolledTextBox
///////////////////////////////////////////////////////////////////////////////
class CScrolledTextBox : public wxScrolledWindow
{
    DECLARE_DYNAMIC_CLASS( CScrolledTextBox )
	public:
        CScrolledTextBox() = default;
		CScrolledTextBox( wxWindow* parent);
        ~CScrolledTextBox();

        void SetValue(const wxString& s);
        virtual void OnEraseBackground(wxEraseEvent& event);

    private:
        int Wrap(const wxString& text, int widthMax, int *lineHeight);
        bool IsStartOfNewLine();
        void OnOutputLine(const wxString& line);

        wxBoxSizer*                 m_TextSizer = nullptr;
        bool                        m_eol = false;
        wxString                    m_text;
        int                         m_hLine = 0;
};



///////////////////////////////////////////////////////////////////////////
/// Class CSlideShowPanel
///////////////////////////////////////////////////////////////////////////////

class CSlideShowPanel : public wxPanel
{
    DECLARE_DYNAMIC_CLASS( CSlideShowPanel )

	public:
        CSlideShowPanel() = default;
		CSlideShowPanel( wxWindow* parent);
		~CSlideShowPanel();

        void OnSlideShowTimer(wxTimerEvent& WXUNUSED(event));
        void SetDescriptionText(void);
        void AdvanceSlideShow(bool changeSlide, bool reload);
        void OnPaint(wxPaintEvent& WXUNUSED(event));
        void OnEraseBackground(wxEraseEvent& event);

    private:
        CScrolledTextBox*           m_description = nullptr;
        wxTimer*                    m_ChangeSlideTimer = nullptr;
        wxBitmap                    m_SlideBitmap;
        bool                        m_bCurrentSlideIsDefault = false;
        bool                        m_bGotAllProjectsList = false;
        bool                        m_bHasBeenDrawn = false;
        ALL_PROJECTS_LIST           m_AllProjectsList;
};


///////////////////////////////////////////////////////////////////////////////
/// Class CSimpleTaskPanel
///////////////////////////////////////////////////////////////////////////////

#ifdef __WXMAC__
#include "MacBitmapComboBox.h"
#else
#define CBOINCBitmapComboBox wxBitmapComboBox
#endif

class CSimpleTaskPanel : public CSimplePanelBase
{
    DECLARE_DYNAMIC_CLASS( CSimpleTaskPanel )

    public:
        CSimpleTaskPanel() = default;
		CSimpleTaskPanel( wxWindow* parent);
		~CSimpleTaskPanel();

        TaskSelectionData* GetTaskSelectionData();
        wxString GetSelectedTaskString() { return m_TaskSelectionCtrl->GetValue(); }
        CBOINCBitmapComboBox* GetTaskSelectionCtrl() { return m_TaskSelectionCtrl; }
        void UpdatePanel(bool delayShow=false);
        void OnTaskSelection(wxCommandEvent &event);
        wxRect* GetProgressRect();
        void ReskinInterface();

	private:
        void GetApplicationAndProjectNames(RESULT* result, wxString* appName, wxString* projName);
        wxString GetElapsedTimeString(double f);
        wxString GetTimeRemainingString(double f);
        wxString GetStatusString(RESULT* result);
        void FindSlideShowFiles(TaskSelectionData *selData);
        void UpdateTaskSelectionList(bool reskin);
        bool isRunning(RESULT* result);
		bool DownloadingResults();
		bool Suspended();
		bool ProjectUpdateScheduled();
		void DisplayIdleState();

	protected:
        wxRect*                     m_progressBarRect = nullptr;
		CTransparentStaticText*     m_myTasksLabel = nullptr;
		CBOINCBitmapComboBox*       m_TaskSelectionCtrl = nullptr;
		CTransparentStaticText*     m_TaskProjectLabel = nullptr;
		CTransparentStaticText*     m_TaskProjectName = nullptr;
#if SELECTBYRESULTNAME
		CTransparentStaticText*     m_TaskApplicationName = nullptr;
#endif
        CSlideShowPanel*            m_SlideShowArea = nullptr;
		CTransparentStaticText*     m_ElapsedTimeValue = nullptr;
		CTransparentStaticText*     m_TimeRemainingValue = nullptr;
		wxGauge*                    m_ProgressBar = nullptr;
		CTransparentStaticText*     m_ProgressValueText = nullptr;
		CTransparentStaticText*     m_StatusValueText = nullptr;
		wxButton*                   m_TaskCommandsButton = nullptr;
        wxRect                      m_ProgressRect;
        int                         m_oldWorkCount = 0;
        int                         m_ipctDoneX1000 = 0;
		time_t                      error_time = 0;
        bool                        m_bStableTaskInfoChanged = false;
        int                         m_CurrentTaskSelection = 0;
        wxString                    m_sNotAvailableString;
        wxString                    m_sNoProjectsString;
};

#endif
