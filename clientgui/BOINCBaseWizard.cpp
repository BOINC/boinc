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
#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "BOINCBaseWizard.h"
#endif

#include "stdwx.h"
#include "diagnostics.h"
#include "util.h"
#include "mfile.h"
#include "miofile.h"
#include "parse.h"
#include "error_numbers.h"
#include "wizardex.h"
#include "error_numbers.h"
#include "BOINCGUIApp.h"
#include "SkinManager.h"
#include "MainDocument.h"
#include "BOINCWizards.h"
#include "BOINCBaseWizard.h"


/*!
 * CBOINCBaseWizard type definition
 */
 
IMPLEMENT_DYNAMIC_CLASS( CBOINCBaseWizard, wxWizardEx )
 
/*!
 * CBOINCBaseWizard constructors
 */
 
CBOINCBaseWizard::CBOINCBaseWizard() :
    wxWizardEx()
{
    IsAttachToProjectWizard = false;
    IsAccountManagerWizard = false;
    IsAccountManagerUpdateWizard = false;
    close_when_completed = false;
}

CBOINCBaseWizard::CBOINCBaseWizard(wxWindow *parent, int id, const wxString& title, const wxPoint& pos, long style) :
    wxWizardEx(parent, id, title, pos, style)
{
    IsAttachToProjectWizard = false;
    IsAccountManagerWizard = false;
    IsAccountManagerUpdateWizard = false;
    close_when_completed = false;
}

/*!
 * Determine if the wizard page has a previous page
 */
wxWizardPageEx* CBOINCBaseWizard::PopPageTransition()
{
    return _PopPageTransition();
}

wxWizardPageEx* CBOINCBaseWizard::_PopPageTransition()
{
    return NULL;
}

/*!
 * Remove the page transition to the stack
 */
wxWizardPageEx* CBOINCBaseWizard::PushPageTransition( wxWizardPageEx* pCurrentPage, unsigned long ulPageID )
{
    return _PushPageTransition( pCurrentPage, ulPageID );
}

wxWizardPageEx* CBOINCBaseWizard::_PushPageTransition( wxWizardPageEx* WXUNUSED(pCurrentPage), unsigned long WXUNUSED(ulPageID) )
{
    return NULL;
}

/*!
 * Process Cancel Event
 */
bool CBOINCBaseWizard::IsCancelInProgress() const
{ 
    return m_bCancelInProgress;
}

void CBOINCBaseWizard::ProcessCancelEvent( wxWizardExEvent& event )
{
    _ProcessCancelEvent( event );
}

void CBOINCBaseWizard::_ProcessCancelEvent( wxWizardExEvent& WXUNUSED(event) )
{
}

/*!
 * Button Controls
 */

wxButton* CBOINCBaseWizard::GetNextButton() const { 
    return m_btnNext;
}

void CBOINCBaseWizard::SimulateNextButton() {
    wxCommandEvent event(wxEVT_COMMAND_BUTTON_CLICKED, m_btnNext->GetId());
    event.SetEventObject(GetNextButton());
    AddPendingEvent(event);
}

void CBOINCBaseWizard::EnableNextButton() {
    m_btnNext->Enable();
}

void CBOINCBaseWizard::DisableNextButton() {
    m_btnNext->Disable();
}

wxButton* CBOINCBaseWizard::GetBackButton() const {
    return m_btnPrev;
}

void CBOINCBaseWizard::SimulateBackButton() {
    wxCommandEvent event(wxEVT_COMMAND_BUTTON_CLICKED, m_btnPrev->GetId());
    event.SetEventObject(GetNextButton());
    AddPendingEvent(event);
}
 
void CBOINCBaseWizard::EnableBackButton() {
    m_btnPrev->Enable();
}

void CBOINCBaseWizard::DisableBackButton() {
    m_btnPrev->Disable();
}

wxButton* CBOINCBaseWizard::GetCancelButton() const {
    return m_btnCancel;
}

void CBOINCBaseWizard::SimulateCancelButton() {
    wxCommandEvent event(wxEVT_COMMAND_BUTTON_CLICKED, m_btnCancel->GetId());
    event.SetEventObject(GetCancelButton());
    AddPendingEvent(event);
}
 
void CBOINCBaseWizard::EnableCancelButton() {
    m_btnCancel->Enable();
}

void CBOINCBaseWizard::DisableCancelButton() {
    m_btnCancel->Disable();
}

