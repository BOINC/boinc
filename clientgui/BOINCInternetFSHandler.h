/////////////////////////////////////////////////////////////////////////////
// Name:        fs_mem.h
// Purpose:     in-memory file system
// Author:      Vaclav Slavik
// Copyright:   (c) 2000 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

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
    void ShutDown();

protected:
    static bool CheckHash(const wxString& strLocation);
    static wxHashTable *m_Hash;
    wxInputStream *m_InputStream;
};

#endif // _WX_FS_CACHEDINTERNET_H_

