/////////////////////////////////////////////////////////////////////////////
// Name:        fs_mem.cpp
// Purpose:     in-memory file system
// Author:      Vaclav Slavik
// RCS-ID:      $Id: fs_mem.cpp 46522 2007-06-18 18:37:40Z VS $
// Copyright:   (c) 2000 Vaclav Slavik
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "BOINCInternetFSHandler.h"
#endif

#include "stdwx.h"
#include "BOINCInternetFSHandler.h"

class MemFSHashObj : public wxObject
{
public:
    MemFSHashObj(wxInputStream* stream, const wxString& mime)
    {
        wxMemoryOutputStream out;
        stream->Read(out);
        m_Len = out.GetSize();
        m_Data = new char[m_Len];
        out.CopyTo(m_Data, m_Len);
        m_MimeType = mime;
        m_Time = wxDateTime::Now();
    }

    virtual ~MemFSHashObj()
    {
        delete[] m_Data;
    }

    char *m_Data;
    size_t m_Len;
    wxString m_MimeType;
    wxDateTime m_Time;

    DECLARE_NO_COPY_CLASS(MemFSHashObj)
};


wxHashTable *CBOINCInternetFSHandler::m_Hash = NULL;


CBOINCInternetFSHandler::CBOINCInternetFSHandler() : wxFileSystemHandler()
{
    if (!m_Hash)
    {
        m_Hash = new wxHashTable(wxKEY_STRING);
    }
}


CBOINCInternetFSHandler::~CBOINCInternetFSHandler()
{
    // as only one copy of FS handler is supposed to exist, we may silently
    // delete static data here. (There is no way how to remove FS handler from
    // wxFileSystem other than releasing _all_ handlers.)
    if (m_Hash)
    {
        WX_CLEAR_HASH_TABLE(*m_Hash);
        delete m_Hash;
        m_Hash = NULL;
    }
}


static wxString StripProtocolAnchor(const wxString& location)
{
    wxString myloc(location.BeforeLast(wxT('#')));
    if (myloc.empty()) myloc = location.AfterFirst(wxT(':'));
    else myloc = myloc.AfterFirst(wxT(':'));

    // fix malformed url:
    if (!myloc.Left(2).IsSameAs(wxT("//")))
    {
        if (myloc.GetChar(0) != wxT('/')) myloc = wxT("//") + myloc;
        else myloc = wxT("/") + myloc;
    }
    if (myloc.Mid(2).Find(wxT('/')) == wxNOT_FOUND) myloc << wxT('/');

    return myloc;
}


bool CBOINCInternetFSHandler::CanOpen(const wxString& location)
{
    wxString p = GetProtocol(location);
    if ((p == wxT("http")) || (p == wxT("ftp")))
    {
        wxURL url(p + wxT(":") + StripProtocolAnchor(location));
        return (url.GetError() == wxURL_NOERR);
    }
    return false;
}


wxFSFile* CBOINCInternetFSHandler::OpenFile(wxFileSystem& WXUNUSED(fs), const wxString& strLocation)
{
    wxString strMIME;

    if (m_Hash)
    {
        MemFSHashObj* obj = (MemFSHashObj*)m_Hash->Get(strLocation);
        if (obj == NULL)
        {
            wxString right = GetProtocol(strLocation) + wxT(":") + StripProtocolAnchor(strLocation);

            wxURL url(right);
            if (url.GetError() == wxURL_NOERR)
            {
                wxInputStream* s = url.GetInputStream();
                strMIME = url.GetProtocol().GetContentType();
                if (strMIME == wxEmptyString) {
                    strMIME = GetMimeTypeFromExt(strLocation);
                }

                if (s)
                {
                    obj = new MemFSHashObj(s, strMIME);
                    delete s;

                    m_Hash->Put(strLocation, obj);

                    return new wxFSFile (
                        new wxMemoryInputStream(obj->m_Data, obj->m_Len),
                        strLocation,
                        strMIME,
                        GetAnchor(strLocation),
                        obj->m_Time
                    );
                }
            }
        }
        else
        {
            strMIME = obj->m_MimeType;
            if ( strMIME.empty() ) {
                strMIME = GetMimeTypeFromExt(strLocation);
            }

            return new wxFSFile (
                new wxMemoryInputStream(obj->m_Data, obj->m_Len),
                strLocation,
                strMIME,
                GetAnchor(strLocation),
                obj->m_Time
            );
        }
    }
    
    return NULL;
}


bool CBOINCInternetFSHandler::CheckHash(const wxString& strLocation)
{
    if (m_Hash->Get(strLocation) != NULL)
        return false;
    else
        return true;
}

