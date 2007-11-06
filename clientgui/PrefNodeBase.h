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
//
#ifndef _PREFNODEBASE_H_
#define _PREFNODEBASE_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "PrefNodeBase.cpp"
#endif

#include "prefs.h"
#include "ValidateBool.h"
#include "ValidateNumber.h"
#include "ValidateTime.h"


enum PrefNodeType {
    Presets = 7000,
    General,
    Processor,
    ProcessorTimes,
    Network,
    NetworkTimes,
    Memory,
    Disk,
};

enum DayOfWeek { 
    Sunday,
    Monday,
    Tuesday,
    Wednesday,
    Thursday,
    Friday,
    Saturday,
};

DECLARE_EVENT_TYPE(PREF_EVT_CMD_UPDATE, -1)

class CPrefNodeBase;

class CPrefNodeBase : public wxScrolledWindow {

	DECLARE_DYNAMIC_CLASS(CPrefNodeBase)

public:
	CPrefNodeBase(wxWindow* parent = NULL, GLOBAL_PREFS* preferences = NULL);
	virtual ~CPrefNodeBase();

    wxWindow* GetHelpAtPoint(const wxPoint& p);

    static CPrefNodeBase* Create(PrefNodeType nodeType, wxWindow* parent, GLOBAL_PREFS* preferences);

protected:
    class CPrefGroup;
    class CPrefValueBase;

    CPrefGroup*     AddGroup(const wxString& title);
    void            AddPreference(CPrefValueBase* pref);

    GLOBAL_PREFS*   m_preferences;

private:
    class CPrefValueTimeSpan;
    wxBoxSizer* m_groupSizer;
    std::vector<CPrefGroup*> m_groupList;
    std::vector<CPrefValueBase*> m_prefList;

// Nested utility classes:
protected:

    // Group box containing pref widgets.
    class CPrefGroup : public wxStaticBoxSizer {
    public:
        CPrefGroup(wxWindow* win, const wxString& title);
        ~CPrefGroup();

        void AddPreference(CPrefValueBase* pref);

    private:
        CPrefNodeBase* m_page;

    };

    // Base for individual preference widgets. Abstract.
    class CPrefValueBase : public wxPanel {

        DECLARE_DYNAMIC_CLASS(CPrefValueBase)
        DECLARE_EVENT_TABLE()

    public:
        CPrefValueBase(wxWindow* parent = NULL);

        CPrefValueBase(
            wxWindow* parent,
            const wxString& xmlElementName,
            const wxString& helpText
        );

    protected:

        virtual void    OnMouseLeave(wxMouseEvent& event);
        virtual void    OnFocus(wxChildFocusEvent& event);
        void            OnCreate(wxWindowCreateEvent& event);

        wxString        m_xmlElementName;
    };


    class CPrefValueText : public CPrefValueBase {

        DECLARE_DYNAMIC_CLASS(CPrefValueNumber)

    public:
        CPrefValueText(wxWindow* parent = NULL);

        CPrefValueText(
            wxWindow* parent,
            const wxString& xmlElementName,
            const wxString& prompt,
            const wxString& units,
            const wxString& helpText,
            const wxTextValidator& val
        );

    protected:
        wxStaticText*       m_prompt;
        wxStaticText*       m_units;
    };


    class CPrefValueBool : public CPrefValueBase {

        DECLARE_DYNAMIC_CLASS(CPrefValueBool)

    public:
        CPrefValueBool(wxWindow* parent = NULL);

        CPrefValueBool(
            wxWindow* parent,
            const wxString& xmlElementName,
            const wxString& prompt,
            const wxString& helpText,
            const CValidateBool& val
        );

    protected:
        wxCheckBox* m_check;

    };

    class CPrefValueButton : public CPrefValueBase {

        DECLARE_DYNAMIC_CLASS(CPrefValueButton)

    public:
        CPrefValueButton(wxWindow* parent = NULL);

        CPrefValueButton(
            wxWindow* parent,
            const wxString& xmlElementName,
            const wxString& prompt,
            const wxString& title,
            const wxString& helpText,
            wxWindowID id
        );
    };

    class CPrefValueTime : public CPrefValueBase {

        DECLARE_DYNAMIC_CLASS(CPrefValueTime)
        DECLARE_EVENT_TABLE()

    public:
        CPrefValueTime(wxWindow* parent = NULL);

        CPrefValueTime(
            wxWindow* parent,
            const wxString& prompt,
            const wxString& helpText,
            TIME_SPAN* time
        );

        void Update();

    protected:
        void SetState(TIME_SPAN::TimeMode mode);
        void OnBetweenChanged(wxCommandEvent& event);
        void OnTextChanged(wxCommandEvent& event);

        TIME_SPAN* m_time;
        wxTextCtrl* m_startCtrl;
        wxTextCtrl* m_endCtrl;

        wxRadioButton* m_rbAlways;
        wxRadioButton* m_rbNever;
        wxRadioButton* m_rbBetween;
    };

    class CPrefValueWeek : public CPrefValueBase {

        DECLARE_DYNAMIC_CLASS(CPrefValueWeek)

    public:
        CPrefValueWeek(wxWindow* parent = NULL);

        CPrefValueWeek(
            wxWindow* parent,
            const wxString& prompt,
            const wxString& helpText,
            TIME_PREFS* prefs
        );

        void Update();
        void                    OnUpdateUI(wxCommandEvent& event);

    protected:
        TIME_PREFS*             m_prefs;
        CPrefValueTimeSpan*     m_dayWidgets[7];
    };


private:
    class CPrefValueTimeSpan : public wxPanel {

        DECLARE_DYNAMIC_CLASS(CPrefValueTimeSpan)
        DECLARE_EVENT_TABLE()

    public:
        CPrefValueTimeSpan(wxWindow* parent = NULL);

        CPrefValueTimeSpan(
            wxWindow* parent,
            DayOfWeek day,
            TIME_PREFS* prefs
        );

        void Update();
        void UpdateDefault();

    protected:
        void OnUseDefaultChanged(wxCommandEvent& event);

        wxCheckBox* m_check;
        wxTextCtrl*     m_timeText;
        DayOfWeek m_day;
        TIME_PREFS* m_prefs;
        
    };
    // End of nested classes
};


#endif // _PREFNODEBASE_H_

