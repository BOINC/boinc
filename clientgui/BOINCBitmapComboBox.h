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

#ifndef __BOINCBITMAPCOMBOBOX__
#define __BOINCBITMAPCOMBOBOX__

#include <wx/bmpcbox.h>

#define EVT_BOINCBITMAPCOMBOBOX EVT_COMBOBOX

// TODO: Subclass CBOINCBitmapComboBox to be accessible on Windows.  
// Either:
// Add WxBitmapComboBoxAccessible class (like CNoticeListCtrlAccessible 
// for CNoticeListCtrl)
// or 
// simulate bitmap combo box using accessible standard Windows controls 
// (as done for CBOINCBitmapComboBox on Mac)
// TODO: Add wx/bmpcbox.h to stdwx.h

class CBOINCBitmapComboBox : public wxBitmapComboBox 
{
    DECLARE_DYNAMIC_CLASS( CBOINCBitmapComboBox )

public:
    CBOINCBitmapComboBox();

    virtual ~CBOINCBitmapComboBox();

    CBOINCBitmapComboBox(wxWindow *parent, wxWindowID id,
            const wxString& value = wxT(""), 
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            int n = 0, const wxString choices[] = NULL,
            long style = 0,
            const wxValidator& validator = wxDefaultValidator,
            const wxString& name = wxT("combo"));

    void * GetClientData(unsigned int n) const;
    void SetClientData(unsigned int n, void *data);
    int Append(const wxString& item);
    int Append(const wxString& item, const wxBitmap& bitmap);
    int Append(const wxString& item, const wxBitmap& bitmap, void *clientData);
    int Insert(const wxString& item, const wxBitmap& bitmap, unsigned int pos);
    int Insert(const wxString& item, const wxBitmap& bitmap, unsigned int pos, void *clientData);
    void Delete(unsigned int n);
    void Clear();
    
private:
    std::vector<void*>      m_pClientData;
};

#endif //__BOINCBITMAPCOMBOBOX__
