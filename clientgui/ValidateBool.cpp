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
#pragma implementation "ValidateBool.h"
#endif

#include "stdwx.h"
#include "ValidateBool.h"


IMPLEMENT_CLASS(CValidateBoolInverse, CValidateBool)


//bool CValidateBoolInverse::Copy(const CValidateBoolInverse& val) {
//
//    CValidateBool::Copy(val);
//    return true;
//}


bool CValidateBoolInverse::TransferToWindow(void) {

    if (!m_validatorWindow) return false;

    if (m_validatorWindow->IsKindOf(CLASSINFO(wxCheckBox))) {

        wxCheckBox* pControl = (wxCheckBox*) m_validatorWindow;
        if (m_pBool)
        {
            pControl->SetValue(! *m_pBool);
            return true;
        }
    }
    return false;
}


bool CValidateBoolInverse::TransferFromWindow(void) {

    if (!m_validatorWindow) return false;

    if (m_validatorWindow->IsKindOf(CLASSINFO(wxCheckBox))) {

        wxCheckBox* pControl = (wxCheckBox*) m_validatorWindow;
        if (m_pBool)
        {
            *m_pBool = !pControl->GetValue();
            return true;
        }
    }
    return false;
}

