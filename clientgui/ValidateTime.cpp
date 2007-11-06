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
#pragma implementation "ValidateTime.h"
#endif

#include "stdwx.h"
#include "ValidateTime.h"


IMPLEMENT_CLASS(CValidateTime, wxTextValidator)


bool CValidateTime::TransferToWindow(void) {

    if (!m_validatorWindow) return false;

    if (m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl))) {

        wxTextCtrl* pControl = (wxTextCtrl*) m_validatorWindow;
        if (m_time) {
            wxString time = DoubleToTimeString(*m_time);
            pControl->SetValue(time);
            return true;
        }
    }
    return false;
}


bool CValidateTime::TransferFromWindow(void) {

    if (!m_validatorWindow) return false;

    if (m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl))) {

        wxTextCtrl* pControl = (wxTextCtrl*) m_validatorWindow;
        if (m_time) {
            return TimeStringToDouble(pControl->GetValue(), *m_time);
        }
    }
    return false;
}


bool CValidateTime::Validate(wxWindow* parent) {

    bool valid = wxTextValidator::Validate(parent);

    if (m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl))) {

        wxTextCtrl* pControl = (wxTextCtrl*) m_validatorWindow;

        wxString timeStr = pControl->GetValue();

        double hour;
        double minutes;
        timeStr.SubString(0, timeStr.First(':')).ToDouble(&hour);
        timeStr.SubString(timeStr.First(':') + 1, timeStr.Length()).ToDouble(&minutes);
        minutes = minutes / 60.0;
        hour += minutes;
        // No need to check for negatives (no minus sign allowed).
        valid = valid && (minutes < 1.0);
        valid = valid && (hour <= 24.0);

        if (!valid) {
            wxMessageBox(_("Time must be expressed as HH:MM"), _("Format error"),
                wxOK | wxICON_EXCLAMATION, parent);
        }
    }
	return valid;
}


// convert a Timestring HH:MM into a double
bool CValidateTime::TimeStringToDouble(wxString timeStr, double& time) {
	double hour;
	double minutes;
    bool valid;

	timeStr.SubString(0, timeStr.First(':')).ToDouble(&hour);
	timeStr.SubString(timeStr.First(':') + 1, timeStr.Length()).ToDouble(&minutes);
	minutes = minutes / 60.0;

    valid = (minutes < 1.0);
    valid = valid && (hour <= 24.0);

    time = hour + minutes;
	return valid;
}


// convert a double into a timestring HH:MM
wxString CValidateTime::DoubleToTimeString(double dt) {
	int hour = (int)dt;
	int minutes = (int)(60.0 * (dt - hour));
	return wxString::Format(wxT("%02d:%02d"), hour, minutes);
}

//

IMPLEMENT_CLASS(CValidateTimeSpan, wxTextValidator)


bool CValidateTimeSpan::TransferToWindow() {

    if (!m_validatorWindow) return false;

    if (m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl))) {

        wxTextCtrl* pControl = (wxTextCtrl*) m_validatorWindow;
        if (m_prefs) {

            const TIME_SPAN* t = m_prefs->week.get(m_day);
            if (!t) t = m_prefs;
            TIME_SPAN::TimeMode mode = t->mode();
            wxString range;
            switch (mode) {
                case TIME_SPAN::Always:
                    range = _("Always");
                    break;
                case TIME_SPAN::Never:
                    range = _("Never");
                    break;
                default:
                    range
                        << CValidateTime::DoubleToTimeString(t->start_hour)
                        << _(" - ")
                        << CValidateTime::DoubleToTimeString(t->end_hour);
                    break;
            }
            pControl->SetValue(range);
            return true;
        }
    }
    return false;
}


bool CValidateTimeSpan::TransferFromWindow() {

    if (!m_validatorWindow) return false;

    if (m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl))) {

        wxTextCtrl* pControl = (wxTextCtrl*) m_validatorWindow;
        
        if (!pControl->IsEnabled()) {
            m_prefs->week.unset(m_day);
            return true;
        } else {
            wxString timeStr = pControl->GetValue();
            TIME_SPAN* span = new TIME_SPAN();

            if (ParseTimeSpan(timeStr, span)) {
                m_prefs->week.set(m_day, span);
                return true;
            } else {
                delete span;
            }
        }
    }
    return false;
}


// Disregard white space and case.
bool CValidateTimeSpan::Validate(wxWindow* parent) {

    bool valid = false;
    if (m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl))) {

        wxTextCtrl* pControl = (wxTextCtrl*) m_validatorWindow;

        if (!pControl->IsEnabled()) return true;

        wxString timeStr = pControl->GetValue();
        valid = ParseTimeSpan(timeStr, 0);

        if (!valid) {
            wxMessageBox(_("Permitted values are 'Always', 'Never' or\n"
                "start and end times in the form HH:MM - HH:MM"), _("Format error"),
                wxOK | wxICON_EXCLAMATION, parent);
        }
    }
	return valid;
}


bool CValidateTimeSpan::ParseTimeSpan(wxString timeStr, TIME_SPAN* span) {

    bool valid = false;
    double start = 0.0, end = 0.0;

    // Strip out all white space
    wxRegEx ws = wxT("[[:space:]]+");
    ws.ReplaceAll(&timeStr, wxEmptyString);

    // Accept localised and non-localised versions.
    if (timeStr.IsSameAs(_("Always"), false)
        || timeStr.IsSameAs(wxT("Always"), false)) {
            end = 24.0;
            valid = true;
    } else if (timeStr.IsSameAs(_("Never"), false)
        || timeStr.IsSameAs(wxT("Never"), false)) {
            start = 24.0;
            valid = true;
    } else {
        // Regex time!
        wxRegEx spanTemplate = wxT("^([[:digit:]]+:[[:digit:]]+)-([[:digit:]]+:[[:digit:]]+$)");
        if (spanTemplate.Matches(timeStr)) {
            
            wxString startStr = spanTemplate.GetMatch(timeStr, 1);
            wxString endStr = spanTemplate.GetMatch(timeStr, 2);

            valid = CValidateTime::TimeStringToDouble(startStr, start)
                && CValidateTime::TimeStringToDouble(endStr, end);
        }
    }
    if (valid && span) {
        span->start_hour = start;
        span->end_hour = end;
    }
    return valid;
}