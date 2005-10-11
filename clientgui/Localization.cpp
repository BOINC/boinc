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
#pragma implementation "Localization.h"
#endif

#include "stdwx.h"
#include "BOINCGUIApp.h"
#include "Localization.h"


CLocalization::CLocalization() {
    m_strSAHMessageBoardsName =
        _("Message boards");
    m_strSAHMessageBoardsDescription =
        _("Correspond with other users on the SETI@home message boards");
    m_strSAHHelpName =
        _("Help");
    m_strSAHHelpDescription =
        _("Ask questions and report problems");
    m_strSAHYourAccuontName = 
        _("Your account");
    m_strSAHYourAccuontDescription =
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
}

const char *BOINC_RCSID_4632804e37="$Id$";
