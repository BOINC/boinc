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
#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "AlreadyExistsPage.h"
#endif

#include "stdwx.h"
#include "wizardex.h"
#include "error_numbers.h"
#include "BOINCGUIApp.h"
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
}

CBOINCBaseWizard::CBOINCBaseWizard(wxWindow *parent, int id, const wxString& title, const wxBitmap& bitmap, const wxPoint& pos, long style) :
    wxWizardEx(parent, id, title, bitmap, pos, style)
{
    IsAttachToProjectWizard = false;
    IsAccountManagerWizard = false;
}

/*!
 * Set the diagnostics flags.
 */
 
void CBOINCBaseWizard::SetDiagFlags( unsigned long ulFlags )
{
    m_ulDiagFlags = ulFlags;
}
 
/*!
 * Check the desired bitmask against our existing bitmask.
 */

bool CBOINCBaseWizard::IsDiagFlagsSet( unsigned long ulFlags )
{
    if (ulFlags & m_ulDiagFlags) {
        return true;
    }
    return false;
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

wxWizardPageEx* CBOINCBaseWizard::_PushPageTransition( wxWizardPageEx* pCurrentPage, unsigned long ulPageID )
{
    return NULL;
}

/*!
 * Process Cancel Event
 */
void CBOINCBaseWizard::ProcessCancelEvent( wxWizardExEvent& event )
{
    _ProcessCancelEvent( event );
}

void CBOINCBaseWizard::_ProcessCancelEvent( wxWizardExEvent& event )
{
}

/*!
 * Button Controls
 */

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

