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

    // SETI@home
    wxString m_strSAHMessageBoardsName;
    wxString m_strSAHMessageBoardsDescription;
    wxString m_strSAHHelpName;
    wxString m_strSAHHelpDescription;
    wxString m_strSAHYourAccuontName;
    wxString m_strSAHYourAccuontDescription;
    wxString m_strSAHYourPreferencesName;
    wxString m_strSAHYourPreferencesDescription;
    wxString m_strSAHYourResultsName;
    wxString m_strSAHYourResultsDescription;
    wxString m_strSAHYourComputersName;
    wxString m_strSAHYourComputersDescription;
    wxString m_strSAHYourTeamName;
    wxString m_strSAHYourTeamDescription;

    // Einstein@home
    wxString m_strEAHCommonQuestionsName;
    wxString m_strEAHCommonQuestionsDesc;
    wxString m_strEAHSceensaverInfoName;
    wxString m_strEAHSceensaverInfoDesc;
    wxString m_strEAHMessageBoardsName;
    wxString m_strEAHMessageBoardsDesc;
    wxString m_strEAHEinsteinStatusName;
    wxString m_strEAHEinsteinStatusDesc;
    wxString m_strEAHReportProblemsName;
    wxString m_strEAHReportProblemsDesc;
    wxString m_strEAHAccountSummaryName;
    wxString m_strEAHAccountSummaryDesc;
    wxString m_strEAHYourAccountName;
    wxString m_strEAHYourAccountDesc;
    wxString m_strEAHYourResultsName;
    wxString m_strEAHYourResultsDescription;
    wxString m_strEAHYourComputersName;
    wxString m_strEAHYourComputersDesc;
    wxString m_strEAHYourTeamName;
    wxString m_strEAHYourTeamDescription;
    wxString m_strEAHLIGOProjectName;
    wxString m_strEAHLIGOProjectDesc;
    wxString m_strEAHGEO600ProjectName;
    wxString m_strEAHGEO600ProjectDesc;

    // Predictor@home
    wxString m_strPAHYourAccountName;
    wxString m_strPAHYourAccountDesc;
    wxString m_strPAHTeamName;
    wxString m_strPAHTeamDesc;

    // climateprediction.net
    wxString m_strCPDNHelpName;
    wxString m_strCPDNHelpDesc;
    wxString m_strCPDNNewsName;
    wxString m_strCPDNNewsDesc;
    wxString m_strCPDNYourAccountName;
    wxString m_strCPDNYourAccountDesc;
    wxString m_strCPDNTeamName;
    wxString m_strCPDNTeamDesc;

    // World Community Grid
    wxString m_strWCGHelpName;
    wxString m_strWCGHelpDesc;
    wxString m_strWCGGlobalStatsName;
    wxString m_strWCGGlobalStatsDesc;
    wxString m_strWCGMyGridName;
    wxString m_strWCGMyGridDesc;
    wxString m_strWCGYourStatsName;
    wxString m_strWCGYourStatsDesc;
    wxString m_strWCGResearchName;
    wxString m_strWCGResearchDesc;
};


#endif

