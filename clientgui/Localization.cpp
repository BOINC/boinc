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
#pragma implementation "Localization.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "Localization.h"


CLocalization::CLocalization() {

	// SETI@home
    m_strSAHMessageBoardsName =
        _("Message boards");
    m_strSAHMessageBoardsDescription =
        _("Correspond with other users on the SETI@home message boards");
    m_strSAHHelpName =
        _("Help");
    m_strSAHHelpDescription =
        _("Ask questions and report problems");
    m_strSAHYourAccountName =
        _("Your account");
    m_strSAHYourAccountDescription =
        _("View your account information and credit totals");
    m_strSAHYourPreferencesName =
        _("Your preferences");
    m_strSAHYourPreferencesDescription =
        _("View and modify your SETI@home account profile and preferences");
    m_strSAHYourResultsName =
        _("Your results");
    m_strSAHYourResultsDescription =
        _("View your last week (or more) of computational results and work");
    m_strSAHYourComputersName =
        _("Your computers");
    m_strSAHYourComputersDescription =
        _("View a listing of all the computers on which you are running SETI@Home");
    m_strSAHYourTeamName =
        _("Your team");
    m_strSAHYourTeamDescription =
        _("View information about your team");

	// Einstein@home
    m_strEAHCommonQuestionsName =
        _("Common questions");
    m_strEAHCommonQuestionsDesc =
        _("Read the Einstein@Home Frequently Asked Question list");
    m_strEAHScreensaverInfoName =
        _("Screensaver info");
    m_strEAHScreensaverInfoDesc =
        _("Read a detailed description of the Einstein@Home screensaver");
    m_strEAHMessageBoardsName =
        _("Message boards");
    m_strEAHMessageBoardsDesc =
        _("Correspond with admins and other users on the Einstein@Home message boards");
	m_strEAHEinsteinStatusName =
        _("Einstein status");
    m_strEAHEinsteinStatusDesc =
        _("Current status of the Einstein@Home server");
    m_strEAHReportProblemsName =
        _("Report problems");
    m_strEAHReportProblemsDesc =
        _("A link to the Einstein@Home problems and bug reports message board");
    m_strEAHYourAccountName =
        _("Your account");
    m_strEAHYourAccountDesc =
        _("View and modify your Einstein@Home account profile and preferences");
	m_strEAHAccountSummaryName =
        _("Account summary");
    m_strEAHAccountSummaryDesc =
        _("View your account information and credit totals");
    m_strEAHYourResultsName =
        _("Your results");
    m_strEAHYourResultsDescription =
        _("View your last week (or more) of computational results and work");
	m_strEAHYourComputersName =
        _("Your computers");
    m_strEAHYourComputersDesc =
        _("View a listing of all the computers on which you are running Einstein@Home");
    m_strEAHYourTeamName =
        _("Your team");
    m_strEAHYourTeamDescription =
        _("View information about your team");
	m_strEAHLIGOProjectName =
        _("LIGO project");
    m_strEAHLIGOProjectDesc =
        _("The home page of the Laser Interferometer Gravitational-wave Observatory (LIGO) project");
    m_strEAHGEO600ProjectName =
        _("GEO-600 project");
    m_strEAHGEO600ProjectDesc =
        _("The home page of the GEO-600 project");

    // Predictor@home
    m_strPAHYourAccountName =
        _("Your account");
    m_strPAHYourAccountDesc =
        _("View your account information and credit totals");
	m_strPAHTeamName =
        _("Team");
    m_strPAHTeamDesc =
        _("Info about your Team");

    // ClimatePrediction.net
    m_strCPDNHelpName =
        _("Help");
    m_strCPDNHelpDesc =
        _("Get help for climateprediction.net");
	m_strCPDNNewsName =
        _("News");
    m_strCPDNNewsDesc =
        _("climateprediction.net News");
	m_strCPDNYourAccountName =
        _("Your account");
    m_strCPDNYourAccountDesc =
        _("View your account information, credits, and trickles");
	m_strCPDNTeamName =
        _("Team");
    m_strCPDNTeamDesc =
        _("Info about your team");

    // World Community Grid
    m_strWCGHelpName =
        _("Help");
    m_strWCGHelpDesc =
        _("Search for help in our help system");
    m_strWCGGlobalStatsName =
        _("Global Statistics");
    m_strWCGGlobalStatsDesc =
        _("Summary statistics for World Community Grid");
    m_strWCGMyGridName =
        _("My Grid");
    m_strWCGMyGridDesc =
        _("Your statistics and settings");
    m_strWCGYourStatsName =
        _("Device Profiles");
    m_strWCGYourStatsDesc =
        _("Update your device settings");
    m_strWCGResearchName =
        _("Research");
    m_strWCGResearchDesc =
        _("Learn about the projects hosted at World Community Grid");

    // Amicable numbers
    m_strANHomePageName =
        _("Home page");
    m_strANMessageBoardsName =
        _("Message boards");
    m_strANYourAccountName =
        _("Your account");
    m_strANYourTasksName =
        _("Your tasks");
    m_strANYourTeamName =
        _("Your team");

}

