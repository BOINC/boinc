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

#ifndef _VALIDATEBOOL_H_
#define _VALIDATEBOOL_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "ValidateBool.cpp"
#endif

#include "wx/valgen.h"

// wxGenericValidator does all we need for a boolean (checkbox) validator.
// Rename it for convenience and clarity:
typedef wxGenericValidator CValidateBool;

// CValidateBoolInverse reverses the checkbox meaning. Checked == false, Not checked == true.
// Useful when the sense of the backing variable does not match the prompt.
class CValidateBoolInverse : public CValidateBool {

    DECLARE_CLASS(CValidateBool)

public:

    CValidateBoolInverse(bool* val) : CValidateBool(val) {};
    CValidateBoolInverse(const CValidateBoolInverse& val): CValidateBool(val) {};

    ~CValidateBoolInverse() {};

    virtual wxObject* Clone() const { return new CValidateBoolInverse(*this); }
    //virtual bool      Copy(const CValidateBoolInverse& val);

    virtual bool      TransferToWindow();
    virtual bool      TransferFromWindow();
};


#endif // _VALIDATEBOOL_H_

