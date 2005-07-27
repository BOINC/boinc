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

#ifndef _LOCALIZATION_H_
#define _LOCALIZATION_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "Localization.cpp"
#endif


class CLocalization : public wxObject
{
    DECLARE_NO_COPY_CLASS(CLocalization)

public:
    CLocalization();

    wxString m_strMessageBoardsName;
    wxString m_strMessageBoardsDescription;

    wxString m_strHelpName;
    wxString m_strHelpDescription;

    wxString m_strYourAccuontName;
    wxString m_strYourAccuontDescription;

    wxString m_strYourPreferencesName;
    wxString m_strYourPreferencesDescription;

    wxString m_strYourResultsName;
    wxString m_strYourResultsDescription;

    wxString m_strYourComputersName;
    wxString m_strYourComputersDescription;

    wxString m_strYourTeamName;
    wxString m_strYourTeamDescription;
};


#endif

