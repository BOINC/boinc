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

#include "stdwx.h"
#include "BOINCBitmapComboBox.h"

// On Windows, wxBitmapComboBox loses an item's clientData when 
// another item is inserted in front of it.  This subclass works 
// around that bug by keeping the clientData separately.

IMPLEMENT_DYNAMIC_CLASS(CBOINCBitmapComboBox, wxBitmapComboBox)

CBOINCBitmapComboBox::CBOINCBitmapComboBox() {}

CBOINCBitmapComboBox::CBOINCBitmapComboBox(wxWindow *parent, wxWindowID id,
            const wxString& value, 
            const wxPoint& pos,
            const wxSize& size,
            int n, const wxString choices[],
            long style,
            const wxValidator& validator,
            const wxString& name
            ) :
    wxBitmapComboBox(parent, id, value, pos, size, n, 
                        choices, style, validator, name
        )
{
    int i;

    for (i=0; i<n; ++i) {
        m_pClientData.push_back(NULL);
    }
}

CBOINCBitmapComboBox::~CBOINCBitmapComboBox() {
    Clear();
}

void * CBOINCBitmapComboBox::GetClientData(unsigned int n) const {
    if (n < GetCount()) {
        return m_pClientData[n];
    }
    else {
        return NULL;
    }
}


void CBOINCBitmapComboBox::SetClientData(unsigned int n, void *data) {
    if (n < GetCount()) {
        m_pClientData[n] = data;
    }
}


int CBOINCBitmapComboBox::Append(const wxString& item, const wxBitmap& bitmap) {
    m_pClientData.push_back(NULL);
    int n = wxBitmapComboBox::Append(item, bitmap);
    return n;
}


int CBOINCBitmapComboBox::Append(const wxString& item, const wxBitmap& bitmap, void *clientData) {
    m_pClientData.push_back(clientData);
    int n = wxBitmapComboBox::Append(item, bitmap);
    return n;
}


int CBOINCBitmapComboBox::Insert(const wxString& item, const wxBitmap& bitmap, unsigned int pos) {
    std::vector<void*>::iterator insertionPoint = m_pClientData.begin();
    m_pClientData.insert(insertionPoint + pos, NULL);
    int n = wxBitmapComboBox::Insert(item, bitmap, pos);
    return n;
}


int CBOINCBitmapComboBox::Insert(const wxString& item, const wxBitmap& bitmap, unsigned int pos, void *clientData) {
    std::vector<void*>::iterator insertionPoint = m_pClientData.begin();
    m_pClientData.insert(insertionPoint + pos, clientData);
    int n = wxBitmapComboBox::Insert(item, bitmap, pos);
    return n;
}


void CBOINCBitmapComboBox::Delete(unsigned int n) {
    if (n < GetCount()) {
        // Caller must have already deleted the data and set the pointer to NULL
        wxASSERT(!m_pClientData[n]);    
        std::vector<void*>::iterator deletionPoint = m_pClientData.begin();
        m_pClientData.erase(deletionPoint + n);
    }
        
    wxBitmapComboBox::Delete(n);
//    Refresh();
}


void CBOINCBitmapComboBox::Clear() {
    int count = GetCount();
	for(int j = count-1; j >=0; --j) {
        // Caller must have already deleted the data and set the pointer to NULL
        wxASSERT(!m_pClientData[j]);    
        m_pClientData[j] = NULL;
        }
    m_pClientData.clear();
    wxBitmapComboBox::Clear();
}
