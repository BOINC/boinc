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
//
#ifndef _BOINCWIZARDS_H_
#define _BOINCWIZARDS_H_

// Wizard Identifiers
//
#define ID_ATTACHPROJECTWIZARD 10000
#define ID_ATTACHACCOUNTMANAGERWIZARD 10001
#define SYMBOL_CWIZARDATTACHPROJECT_IDNAME ID_ATTACHPROJECTWIZARD
#define SYMBOL_CWIZARDACCOUNTMANAGER_IDNAME ID_ATTACHACCOUNTMANAGERWIZARD

// Page Identifiers
//

// Generic Pages
#define ID_WELCOMEPAGE 10100
#define ID_TERMSOFUSEPAGE 10113
#define ID_ACCOUNTINFOPAGE 10102
#define ID_COMPLETIONPAGE 10103
#define ID_COMPLETIONERRORPAGE 10104
#define ID_ERRNOTDETECTEDPAGE 10105
#define ID_ERRUNAVAILABLEPAGE 10106
#define ID_ERRNOINTERNETCONNECTIONPAGE 10108
#define ID_ERRNOTFOUNDPAGE 10109
#define ID_ERRALREADYEXISTSPAGE 10110
#define ID_ERRPROXYINFOPAGE 10111
#define ID_ERRPROXYPAGE 10112

// Attach to Project Wizard Pages
#define ID_PROJECTINFOPAGE 10200
#define ID_PROJECTPROPERTIESPAGE 10201
#define ID_PROJECTPROCESSINGPAGE 10202

// Account Manager Wizard Pages
#define ID_ACCOUNTMANAGERINFOPAGE 10300
#define ID_ACCOUNTMANAGERPROPERTIESPAGE 10301
#define ID_ACCOUNTMANAGERPROCESSINGPAGE 10302


// Control Identifiers
//

// Bitmap Progress Control
#define ID_PROGRESSCTRL 11000

// BOINC Hyperlink Control
#define ID_BOINCHYPERLINK 11001

// Completion Error Page Multiline Text Control
#define ID_TEXTCTRL 11002

// Change applications button control
#define ID_CHANGEAPPS 11003

// Welcome Page Controls
#define ID_SELECTPROJECTWIZARD 11020
#define ID_SELECTAMWIZARD 11021

// Debug Flag Controls
#define ID_ERRPROJECTPROPERTIES 11100
#define ID_ERRPROJECTCOMM 11101
#define ID_ERRPROJECTPROPERTIESURL 11102
#define ID_ERRACCOUNTCREATIONDISABLED 11103
#define ID_ERRCLIENTACCOUNTCREATIONDISABLED 11104
#define ID_ERRACCOUNTALREADYEXISTS 11105
#define ID_ERRPROJECTALREADYATTACHED 11106
#define ID_ERRPROJECTATTACHFAILURE 11107
#define ID_ERRGOOGLECOMM 11108
#define ID_ERRNETDETECTION 11110

// Project Info/Account Manager Info Controls
#define ID_PROJECTSELECTIONCTRL 11200
#define ID_PROJECTURLSTATICCTRL 11201
#define ID_PROJECTURLDESCRIPTIONSTATICCTRL 11202
#define ID_PROJECTURLCTRL 11203

// Terms Of Use Controls
#define ID_TERMSOFUSECTRL 11300
#define ID_TERMSOFUSEAGREECTRL 11301
#define ID_TERMSOFUSEDISAGREECTRL 11302

// Account Info Controls
#define ID_ACCOUNTCREATECTRL 11400
#define ID_ACCOUNTUSEEXISTINGCTRL 11401
#define ID_ACCOUNTEMAILADDRESSSTATICCTRL 11402
#define ID_ACCOUNTEMAILADDRESSCTRL 11403
#define ID_ACCOUNTPASSWORDSTATICCTRL 11404
#define ID_ACCOUNTPASSWORDCTRL 11405
#define ID_ACCOUNTCONFIRMPASSWORDSTATICCTRL 11406
#define ID_ACCOUNTCONFIRMPASSWORDCTRL 11407
#define ID_ACCOUNTREQUIREMENTSSTATICCTRL 11408
#define ID_ACCOUNTFORGOTPASSWORDCTRL 11409

// Proxy Page Controls
#define ID_PROXYHTTPSERVERSTATICCTRL 11500
#define ID_PROXYHTTPSERVERCTRL 11501
#define ID_PROXYHTTPPORTSTATICCTRL 11502
#define ID_PROXYHTTPPORTCTRL 11503
#define ID_PROXYHTTPUSERNAMESTATICCTRL 11504
#define ID_PROXYHTTPUSERNAMECTRL 11505
#define ID_PROXYHTTPPASSWORDSTATICCTRL 11506
#define ID_PROXYHTTPPASSWORDCTRL 11507
#define ID_PROXYHTTPAUTODETECTCTRL 11508
#define ID_PROXYSOCKSSERVERSTATICCTRL 11509
#define ID_PROXYSOCKSSERVERCTRL 11510
#define ID_PROXYSOCKSPORTSTATICCTRL 11511
#define ID_PROXYSOCKSPORTCTRL 11512
#define ID_PROXYSOCKSUSERNAMESTATICCTRL 11513
#define ID_PROXYSOCKSUSERNAMECTRL 11514
#define ID_PROXYSOCKSPASSWORDSTATICCTRL 11515
#define ID_PROXYSOCKSPASSWORDCTRL 11516

// Account Manager Status Controls
#define ID_ACCTMANAGERNAMECTRL 11600
#define ID_ACCTMANAGERLINKCTRL 11601
#define ID_ACCTMANAGERUPDATECTRL 11602
#define ID_ACCTMANAGERREMOVECTRL 11603


// Forward declare the generic page classes
//
class CWelcomePage;
class CAccountInfoPage;
class CTermsOfUsePage;
class CCompletionPage;
class CCompletionErrorPage;
class CErrNotDetectedPage;
class CErrUnavailablePage;
class CErrAlreadyAttachedPage;
class CErrNoInternetConnectionPage;
class CErrNotFoundPage;
class CErrAlreadyExistsPage;
class CErrProxyInfoPage;
class CErrProxyPage;


// Forward declare special controls
//
class wxHyperLink;


// Diagnostics Tools
//
#define WIZDEBUG_ERRPROJECTPROPERTIES                 0x00000001
#define WIZDEBUG_ERRPROJECTPROPERTIESURL              0x00000002
#define WIZDEBUG_ERRGOOGLECOMM                        0x00000008
#define WIZDEBUG_ERRNETDETECTION                      0x00000010
#define WIZDEBUG_ERRPROJECTCOMM                       0x00000020
#define WIZDEBUG_ERRACCOUNTNOTFOUND                   0x00000040
#define WIZDEBUG_ERRACCOUNTALREADYEXISTS              0x00000080
#define WIZDEBUG_ERRACCOUNTCREATIONDISABLED           0x00000100
#define WIZDEBUG_ERRCLIENTACCOUNTCREATIONDISABLED     0x00000200
#define WIZDEBUG_ERRPROJECTATTACH                     0x00000400
#define WIZDEBUG_ERRPROJECTALREADYATTACHED            0x00000800
#define WIZDEBUG_ERRTERMSOFUSEREQUIRED                0x00001000

#define PROCESS_DEBUG_FLAG(ulFlags) \
    ((CBOINCBaseWizard*)GetParent())->SetDiagFlags(ulFlags)

#define CHECK_DEBUG_FLAG(id) \
    ((CBOINCBaseWizard*)GetParent())->IsDiagFlagsSet(id)

// Wizard Detection
//
#define IS_ATTACHTOPROJECTWIZARD() \
    ((CBOINCBaseWizard*)GetParent())->IsAttachToProjectWizard

#define IS_ACCOUNTMANAGERWIZARD() \
    ((CBOINCBaseWizard*)GetParent())->IsAccountManagerWizard

#define IS_ACCOUNTMANAGERUPDATEWIZARD() \
    ((CBOINCBaseWizard*)GetParent())->IsAccountManagerUpdateWizard

#define IS_ACCOUNTMANAGERREMOVEWIZARD() \
    ((CBOINCBaseWizard*)GetParent())->IsAccountManagerRemoveWizard


// Commonly defined macros
//
#define PAGE_TRANSITION_NEXT(id) \
    ((CBOINCBaseWizard*)GetParent())->PushPageTransition((wxWizardPageEx*)this, id)
 
#define PAGE_TRANSITION_BACK \
    ((CBOINCBaseWizard*)GetParent())->PopPageTransition()
 
#define PROCESS_CANCELEVENT(event) \
    ((CBOINCBaseWizard*)GetParent())->ProcessCancelEvent(event)

#define CHECK_CLOSINGINPROGRESS() \
    ((CBOINCBaseWizard*)GetParent())->IsCancelInProgress()


#endif // _BOINCWIZARDS_H_

