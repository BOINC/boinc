// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2012 University of California
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

#ifndef _WX_FS_CACHEDINTERNET_H_
#define _WX_FS_CACHEDINTERNET_H_

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma interface "BOINCInternetFSHandler.cpp"
#endif

class CBOINCInternetFSHandler : public wxFileSystemHandler
{
public:
    CBOINCInternetFSHandler();
    virtual ~CBOINCInternetFSHandler();

    virtual bool CanOpen(const wxString& strLocation);
    virtual wxFSFile* OpenFile(wxFileSystem& fs, const wxString& strLocation);
    void UnchacheMissingItems();
    void ClearCache();
    void ShutDown();

protected:
    static bool CheckHash(const wxString& strLocation);
    static wxHashTable *m_Hash;
    wxInputStream *m_InputStream;
    
private:
    bool m_bMissingItems;
};

#endif // _WX_FS_CACHEDINTERNET_H_

