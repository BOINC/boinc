/////////////////////////////////////////////////////////////////////////////
// Name:        src/msw/urlmsw.cpp
// Purpose:     MS-Windows native URL support based on WinINet
// Author:      Hajo Kirchhoff
// Modified by:
// Created:     06/11/2003
// RCS-ID:      $Id: urlmsw.cpp 58116 2009-01-15 12:45:22Z VZ $
// Copyright:   (c) 2003 Hajo Kirchhoff
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// Modified for BOINC from wxWidgets 2.8.10 files src/common/fs_mem.cpp and
// src/msw/urlmsw.cpp

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "BOINCInternetFSHandler.h"
#endif

#include "stdwx.h"
#include "BOINCInternetFSHandler.h"
#include "BOINCGUIApp.h"
#include "MainDocument.h"
#include "util.h"


class MemFSHashObj : public wxObject
{
public:
    MemFSHashObj(wxInputStream* stream, const wxString& mime, const wxString& key)
    {
        if (stream) {
            wxMemoryOutputStream out;
            stream->Read(out);
            m_Len = out.GetSize();
            m_Data = new char[m_Len];
            out.CopyTo(m_Data, m_Len);
        } else {
            m_Len = 0;
            m_Data = NULL;
        }
        m_Key = key;
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
    wxString m_Key;

    DECLARE_NO_COPY_CLASS(MemFSHashObj)
};


wxHashTable *CBOINCInternetFSHandler::m_Hash = NULL;

static bool b_ShuttingDown = false;

#ifdef __WXMSW__
// *** code adapted from src/msw/urlmsw.cpp (wxWidgets 2.8.10)

// If OpenURL fails, we probably don't have a connection to 
// the Internet, so use a shorter timeout for subsequent calls
// to OpenURL until one succeeds.
// Otherwise notices takes too long to display if there are
// multiple notices with images.

#define STANDARD_INTERNET_TIMEOUT 5
#define SHORT_INTERNET_TIMEOUT 2
static double dInternetTimeout = STANDARD_INTERNET_TIMEOUT;

#ifdef __VISUALC__  // be conservative about this pragma
    // tell the linker to include wininet.lib automatically
    #pragma comment(lib, "wininet.lib")
#endif

#include "wx/url.h"

#include <string.h>
#include <ctype.h>
#include <wininet.h>

// this class needn't be exported
class wxWinINetURL
{
public:
    wxInputStream *GetInputStream(wxURL *owner);

protected:
    // return the WinINet session handle
    static HINTERNET GetSessionHandle(bool closeSessionHandle = false);
    
};

////static bool lastReadHadEOF = false;
static bool operationEnded = false;
static DWORD lastInternetStatus;
static LPVOID lastlpvStatusInformation;
static DWORD lastStatusInfo;
static DWORD lastStatusInfoLength;

// Callback for InternetOpenURL() and InternetReadFileEx()
static void CALLBACK BOINCInternetStatusCallback(
                    HINTERNET,
                    DWORD_PTR,
                    DWORD dwInternetStatus,
                    LPVOID lpvStatusInformation,
                    DWORD dwStatusInformationLength
            )
{
    lastInternetStatus = dwInternetStatus;
    lastlpvStatusInformation = lpvStatusInformation;
    lastStatusInfoLength = dwStatusInformationLength;
    if (lastStatusInfoLength == sizeof(DWORD)) {
        lastStatusInfo = *(DWORD*)lpvStatusInformation;
    }
    switch (dwInternetStatus) {
    case INTERNET_STATUS_REQUEST_COMPLETE:
        operationEnded = true;
        break;
    case INTERNET_STATUS_STATE_CHANGE:
        if (lastStatusInfo & (INTERNET_STATE_DISCONNECTED | INTERNET_STATE_DISCONNECTED_BY_USER)) {
            operationEnded = true;
        }
        break;
    }
}


HINTERNET wxWinINetURL::GetSessionHandle(bool closeSessionHandle)
{
    // this struct ensures that the session is opened when the
    // first call to GetSessionHandle is made
    // it also ensures that the session is closed when the program
    // terminates
    static struct INetSession
    {
        INetSession()
        {
            INetOpenSession();
        }

        ~INetSession()
        {
            INetCloseSession();
        }

        void INetOpenSession() {
            DWORD rc = InternetAttemptConnect(0);

            m_handle = InternetOpen
                       (
                        wxVERSION_STRING,
                        INTERNET_OPEN_TYPE_PRECONFIG,
                        NULL,
                        NULL,
                        INTERNET_FLAG_ASYNC |
                            (rc == ERROR_SUCCESS ? 0 : INTERNET_FLAG_OFFLINE)
                       );

            if (m_handle) {
                InternetSetStatusCallback(m_handle, BOINCInternetStatusCallback);
            }
        }
        
        void INetCloseSession() {
            InternetSetStatusCallback(NULL, BOINCInternetStatusCallback);
            
            while (m_handle) {
                BOOL closedOK = InternetCloseHandle(m_handle);
                if (closedOK) {
                    m_handle = NULL;
                } else {
                    wxGetApp().Yield(true);
                }
            }
        }
    
        HINTERNET m_handle;
    } session;
    
     CMainDocument* pDoc      = wxGetApp().GetDocument();

    wxASSERT(pDoc);

    if (closeSessionHandle) {
        while (session.m_handle) {
            BOOL closedOK = InternetCloseHandle(session.m_handle);
            if (closedOK) {
                session.m_handle = NULL;
            } else{
                wxGetApp().Yield(true);
            }
        }
        return 0;
    }

    if (!session.m_handle) {
        session.INetOpenSession();
    }
    return session.m_handle;
}
\


// this class needn't be exported
class /*WXDLLIMPEXP_NET */ wxWinINetInputStream : public wxInputStream
{
public:
    wxWinINetInputStream(HINTERNET hFile=0);
    virtual ~wxWinINetInputStream();

    void Attach(HINTERNET hFile);

    wxFileOffset SeekI( wxFileOffset WXUNUSED(pos), wxSeekMode WXUNUSED(mode) )
        { return -1; }
    wxFileOffset TellI() const
        { return -1; }
    size_t GetSize() const;

protected:
    void SetError(wxStreamError err) { m_lasterror=err; }
    HINTERNET m_hFile;
    size_t OnSysRead(void *buffer, size_t bufsize);

    DECLARE_NO_COPY_CLASS(wxWinINetInputStream)
};


size_t wxWinINetInputStream::GetSize() const
{
    DWORD contentLength = 0;
    DWORD dwSize = sizeof(contentLength);
    DWORD index = 0;

    if (!m_hFile) {
        return 0;
    }
    if ( HttpQueryInfo( m_hFile, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &contentLength, &dwSize, &index) )
        return contentLength;
    else
        return 0;
}


size_t wxWinINetInputStream::OnSysRead(void *buffer, size_t bufsize)
{
    DWORD bytesread = 0;
    DWORD lError = ERROR_SUCCESS;
    INTERNET_BUFFERS bufs;
    BOOL success = false;
    CMainDocument* pDoc      = wxGetApp().GetDocument();

    wxASSERT(pDoc);

    if (b_ShuttingDown || (!pDoc->IsConnected())) {
        SetError(wxSTREAM_EOF);
        return 0;
    }

    if (!m_hFile) {
        SetError(wxSTREAM_READ_ERROR);
        return 0;
    }
    
    memset(&bufs, 0, sizeof(bufs));
    bufs.dwStructSize = sizeof(INTERNET_BUFFERS);
    bufs.Next = NULL;
    bufs.lpvBuffer = buffer;
    bufs.dwBufferLength = (DWORD)bufsize;

    success = InternetReadFileEx(m_hFile, &bufs, IRF_SYNC, 2);
    
    lError = ::GetLastError();
    
#if 0       // Possibly useful for debugging
    if ((!success) || (lError != ERROR_SUCCESS)) {
        DWORD iError, bLength = 0;
        InternetGetLastResponseInfo(&iError, NULL, &bLength);
        if ( bLength > 0 )
        {
            wxString errorString;
            InternetGetLastResponseInfo
            (
                &iError,
                wxStringBuffer(errorString, bLength),
                &bLength
            );

            wxLogError(wxT("Read failed with error %d: %s"),
                    iError, errorString.c_str());
        }
#endif

    if (!success) {
        return 0;
    }

    bytesread = bufs.dwBufferLength;
    if (lError != ERROR_SUCCESS) {
        SetError(wxSTREAM_READ_ERROR);
    } else {
        if ( bytesread == 0 )
        {
            SetError(wxSTREAM_EOF);
        }
    }
    
    return bytesread;
}


wxWinINetInputStream::wxWinINetInputStream(HINTERNET hFile)
                    : m_hFile(hFile)
{
}


void wxWinINetInputStream::Attach(HINTERNET newHFile)
{
    wxCHECK_RET(m_hFile==NULL,
        wxT("cannot attach new stream when stream already exists"));
    m_hFile=newHFile;
    SetError(m_hFile!=NULL ? wxSTREAM_NO_ERROR : wxSTREAM_READ_ERROR);
}


wxWinINetInputStream::~wxWinINetInputStream()
{
    if ( m_hFile )
    {
        InternetCloseHandle(m_hFile);
        m_hFile=0;
    }
}


wxInputStream *wxWinINetURL::GetInputStream(wxURL *owner)
{
static bool bAlreadyRunning = false;
    if (bAlreadyRunning) {
        fprintf(stderr, "wxWinINetURL::GetInputStream reentered!");
        return NULL;
    }
    bAlreadyRunning = true;
    DWORD service;
    CMainDocument* pDoc      = wxGetApp().GetDocument();

    wxASSERT(pDoc);

    if (b_ShuttingDown || (!pDoc->IsConnected())) {
        GetSessionHandle(true); // Closes the session handle
        bAlreadyRunning = false;
        return 0;
    }
    
    if ( owner->GetScheme() == wxT("http") )
    {
        service = INTERNET_SERVICE_HTTP;
    }
    else if ( owner->GetScheme() == wxT("ftp") )
    {
        service = INTERNET_SERVICE_FTP;
    }
    else
    {
        bAlreadyRunning = false;
        // unknown protocol. Let wxURL try another method.
        return 0;
    }
    
    wxWinINetInputStream *newStream = new wxWinINetInputStream;
    
    operationEnded = false;
    double endtimeout = dtime() + dInternetTimeout;

    HINTERNET newStreamHandle = InternetOpenUrl
                                (
                                    GetSessionHandle(),
                                    owner->GetURL(),
                                    NULL,
                                    0,
                                    INTERNET_FLAG_KEEP_CONNECTION |
                                    INTERNET_FLAG_PASSIVE,
                                    1
                                );
                              
    while (!operationEnded) {
        if (b_ShuttingDown || 
            (!pDoc->IsConnected()) || 
            (dtime() > endtimeout)
            ) {
            GetSessionHandle(true); // Closes the session handle
            if (newStreamHandle) {
                newStreamHandle = NULL;
            }
            if (newStream) {
                delete newStream;
                newStream = NULL;
            }
            dInternetTimeout = SHORT_INTERNET_TIMEOUT;
            bAlreadyRunning = false;
            return 0;
        }
        
        wxGetApp().Yield(true);
    }

    if ((lastInternetStatus == INTERNET_STATUS_REQUEST_COMPLETE) &&
            (lastStatusInfoLength >= sizeof(HINTERNET)) &&
            (!b_ShuttingDown)
        ) {
            INTERNET_ASYNC_RESULT* res = (INTERNET_ASYNC_RESULT*)lastlpvStatusInformation;
            if (res && !res->dwError) {
                newStreamHandle = (HINTERNET)(res->dwResult);
            } else {
                newStreamHandle = NULL;
            }
    }
    
    if (!newStreamHandle) {
        GetSessionHandle(true); // Closes the session handle
        dInternetTimeout = SHORT_INTERNET_TIMEOUT;
        bAlreadyRunning = false;
        return NULL;
    }

    newStream->Attach(newStreamHandle);

    dInternetTimeout = STANDARD_INTERNET_TIMEOUT;
    bAlreadyRunning = false;
    return newStream;
}

// *** End of code adapted from src/msw/urlmsw.cpp (wxWidgets 2.8.10)
#endif // __WXMSW__


CBOINCInternetFSHandler::CBOINCInternetFSHandler() : wxFileSystemHandler()
{
    m_InputStream = NULL;
    b_ShuttingDown = false;
    m_bMissingItems = false;
    
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
    if (b_ShuttingDown) return false;

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

    if (b_ShuttingDown) return NULL;

    if (m_Hash)
    {
        MemFSHashObj* obj = (MemFSHashObj*)m_Hash->Get(strLocation);
        if (obj == NULL)
        {
            wxString right = GetProtocol(strLocation) + wxT(":") + StripProtocolAnchor(strLocation);

            wxURL url(right);
            if (url.GetError() == wxURL_NOERR)
            {
#ifdef __WXMSW__
                wxWinINetURL * winURL = new wxWinINetURL;
                m_InputStream = winURL->GetInputStream(&url);
#else
                m_InputStream = url.GetInputStream();
#endif
                strMIME = url.GetProtocol().GetContentType();
                if (strMIME == wxEmptyString) {
                    strMIME = GetMimeTypeFromExt(strLocation);
                }

                obj = new MemFSHashObj(m_InputStream, strMIME, strLocation);
                delete m_InputStream;
                m_InputStream = NULL;

                m_Hash->Put(strLocation, obj);
                
                // If we couldn't read image, then return NULL so 
                // image tag handler displays "broken image" bitmap
                if (obj->m_Len == 0) {
                    m_bMissingItems = true;
                    return NULL;
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
        else
        {
            strMIME = obj->m_MimeType;
            if ( strMIME.empty() ) {
                strMIME = GetMimeTypeFromExt(strLocation);
            }

            // If we couldn't read image, then return NULL so 
            // image tag handler displays "broken image" bitmap
            if (obj->m_Len == 0) {
                return NULL;
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


void CBOINCInternetFSHandler::UnchacheMissingItems() {
    m_Hash->BeginFind();
    wxHashTable::Node* node = m_Hash->Next();
    for(;;) {
        if (node == NULL) break;   // End of cache
        MemFSHashObj* obj = (MemFSHashObj*)node->GetData();
        // We must get next node before deleting this one
        node = m_Hash->Next();
        if (obj->m_Len == 0) {
            delete m_Hash->Delete(obj->m_Key);
        }
    }
    m_bMissingItems = false;
#ifdef __WXMSW__
    dInternetTimeout = STANDARD_INTERNET_TIMEOUT;
#endif
}


void CBOINCInternetFSHandler::ClearCache() {
    WX_CLEAR_HASH_TABLE(*m_Hash);
    m_bMissingItems = false;
#ifdef __WXMSW__
    dInternetTimeout = STANDARD_INTERNET_TIMEOUT;
#endif
}


void CBOINCInternetFSHandler::ShutDown() {
    b_ShuttingDown = true;
#ifdef __WXMSW__
    if (m_InputStream) {
        delete m_InputStream;
        m_InputStream = NULL;
    }
#endif
}
